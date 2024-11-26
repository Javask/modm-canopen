#ifndef CANOPEN_CANOPEN_DEVICE_HPP
#error "Do not include this file directly, use canopen_device.hpp instead"
#endif
#include "../nmt_command.hpp"
namespace modm_canopen
{

template<typename OD, typename... Protocols>
auto
CanopenDevice<OD, Protocols...>::write(Address address, Value value) -> SdoErrorCode
{
	auto entry = OD::map.lookup(address);
	if (!entry) { return SdoErrorCode::ObjectDoesNotExist; }
	if (value.index() != static_cast<uint32_t>(entry->dataType))
	{
		return SdoErrorCode::UnsupportedAccess;
	}

	auto handler = accessHandlers.lookupWriteHandler(address);
	if (handler)
	{
		const auto result = callWriteHandler(*handler, value);
		if (result == SdoErrorCode::NoError) { setValueChanged(address); }
		return result;
	} else if (!entry->isWritable())
	{
		return SdoErrorCode::WriteOfReadOnlyObject;
	} else
	{  // TODO: can this happen?
		return SdoErrorCode::UnsupportedAccess;
	}
}

template<typename OD, typename... Protocols>
std::optional<Value>
CanopenDevice<OD, Protocols...>::toValue(Address address, std::span<const uint8_t> data,
										 int8_t size)
{
	auto entry = OD::map.lookup(address);
	if (!entry) { return {}; }
	if (!entry->isWritable()) { return {}; }

	const auto objectSize = getDataTypeSize(entry->dataType);
	const bool sizeIsValid =
		(objectSize <= data.size()) && ((size == -1) || (size == int8_t(objectSize)));
	if (!sizeIsValid) { return {}; }
	return valueFromBytes(entry->dataType, data.data());
}

template<typename OD, typename... Protocols>
auto
CanopenDevice<OD, Protocols...>::write(Address address, std::span<const uint8_t> data,
									   int8_t size) -> SdoErrorCode
{
	auto entry = OD::map.lookup(address);
	if (!entry)
	{
		if (address.subindex != 0 && OD::map.lookup(Address{.index = address.index, .subindex = 0}))
			return SdoErrorCode::SubIndexDoesNotExist;
		return SdoErrorCode::ObjectDoesNotExist;
	}
	if (!entry->isWritable()) { return SdoErrorCode::WriteOfReadOnlyObject; }

	const auto objectSize = getDataTypeSize(entry->dataType);
	const bool sizeIsValid =
		(objectSize <= data.size()) && ((size == -1) || (size == int8_t(objectSize)));
	if (!sizeIsValid) { return SdoErrorCode::UnsupportedAccess; }

	auto handler = accessHandlers.lookupWriteHandler(address);
	if (handler)
	{
		const Value value = valueFromBytes(entry->dataType, data);
		const auto result = callWriteHandler(*handler, value);
		if (result == SdoErrorCode::NoError) { setValueChanged(address); }

		return result;
	}
	return SdoErrorCode::UnsupportedAccess;
}

template<typename OD, typename... Protocols>
auto
CanopenDevice<OD, Protocols...>::read(Address address) -> std::variant<Value, SdoErrorCode>
{
	auto handler = accessHandlers.lookupReadHandler(address);
	if (handler)
	{
		return callReadHandler(*handler);
	} else
	{
		auto entry = OD::map.lookup(address);
		if (!entry)
		{
			if (address.subindex != 0 &&
				OD::map.lookup(Address{.index = address.index, .subindex = 0}))
				return SdoErrorCode::SubIndexDoesNotExist;
			return SdoErrorCode::ObjectDoesNotExist;
		} else if (entry->isReadable())
		{
			return SdoErrorCode::ReadOfWriteOnlyObject;
		}
	}
	return SdoErrorCode::UnsupportedAccess;
}

template<typename OD, typename... Protocols>
void
CanopenDevice<OD, Protocols...>::handleNMTCommand(const modm::can::Message& msg)
{
	if (msg.getLength() != 2)
	{
		// Invalid length
		return;
	}
	if (msg.data[1] != nodeId_ && msg.data[1] != 0)
	{
		// Not for us
		return;
	}
	if (!isValidNMTCommand(msg.data[0]))
	{
		// Invalid command
		return;
	}

	const auto oldState = state_;
	NMTCommand cmd = (NMTCommand)msg.data[0];
	switch (cmd)
	{
		case NMTCommand::Start:
			state_ = NMTState::Operational;
			break;
		case NMTCommand::Stop:
			state_ = NMTState::Stopped;
			break;
		case NMTCommand::ResetNode:
			// TODO reset for real
			state_ = NMTState::PreOperational;
			break;
		case NMTCommand::EnterPreOperational:
			state_ = NMTState::PreOperational;
			break;
		default:
			break;
	}

	if (oldState != state_)
	{
		MODM_LOG_INFO << "NMT State changed: " << nmtStateToString(state_) << modm::endl;
	}
}

template<typename OD, typename... Protocols>
void
CanopenDevice<OD, Protocols...>::handleSync(const modm::can::Message& message)
{
	if (syncPeriod_.count() == 0) return;  // SYNC is disabled
	if ((syncCounterOverflow_ == 0 && message.getLength() != 0) ||
		(syncCounterOverflow_ != 0 && message.getLength() != 1))
	{
		// Wrong packet size
		setError(EMCYError::UnexpectedSYNCData);
	}
	if (syncCounterOverflow_ != 0)
	{
		const uint8_t newCounter = message.data[0];
		int diff = (newCounter - lastSyncCounter_) % syncCounterOverflow_;
		if (diff < 0) diff += syncCounterOverflow_;

		if (diff > 1 || newCounter > syncCounterOverflow_)
		{
			// Wrong SYNC counter value
			setError(EMCYError::UnexpectedSYNCData);
		}
		lastSyncCounter_ = newCounter;
	}
	const auto now = modm::PreciseClock::now();
	const auto deviation = (int)(now - (lastSyncTime_ + syncPeriod_)).count();
	if (lastSyncTime_.time_since_epoch().count() != 0 && syncPeriod_.count() != 0 &&
		deviation > 1000)  // More than a millisecond inaccurate
	{
		MODM_LOG_DEBUG << "Got SYNC with deviation " << deviation << "us" << modm::endl;
		setError(EMCYError::GenericCommunicationError);
	} else
	{
		// Reset after one packet on time
		missedSync_ = false;
	}
	lastSyncTime_ = now;

	for (auto& tpdo : transmitPdos_) { tpdo.sync(); }
}

template<typename OD, typename... Protocols>
template<typename MessageCallback>
void
CanopenDevice<OD, Protocols...>::sendEMCY(MessageCallback&& cb)
{
	if (emcyEnabled_)
	{
		emcyDue_ = false;
		lastEmcyTime_ = modm::PreciseClock::now();
		modm::can::Message msg{};
		msg.setIdentifier(emcyCobId_);
		msg.setExtended(false);
		*((uint16_t*)msg.data) = (uint16_t)emcy_;
		msg.data[2] = errorReg_;
		std::copy(manufacturerError_.begin(), manufacturerError_.end(),
				  std::span<uint8_t>(msg.data + 3, msg.capacity - 3).begin());
		msg.setLength(3 + manufacturerError_.size());
		cb(msg);
	}
}

template<typename OD, typename... Protocols>
template<typename MessageCallback>
void
CanopenDevice<OD, Protocols...>::processMessage(const modm::can::Message& message,
												MessageCallback&& cb)
{
	if (message.getIdentifier() == syncCobId_)
	{
		handleSync(message);
		return;
	}
	Heartbeat<CanopenDevice>::processMessage(message, std::forward<MessageCallback>(cb));
	if (message.getIdentifier() == 0)
	{
		handleNMTCommand(message);
		return;
	}

	if (state_ != NMTState::Stopped)
	{
		if ((message.identifier & 0x7f) == nodeId_)
		{
			SdoServer<CanopenDevice>::processMessage(message, std::forward<MessageCallback>(cb));
		}
	}
	if (state_ == NMTState::Operational)
	{
		for (auto& rpdo : receivePdos_)
		{
			if (rpdo.getTransmitMode().isAsync() ||
				(rpdo.getTransmitMode().isOnSync() && isInSyncWindow()))
			{
				rpdo.processMessage(message,
									[](Address address, Value value) { write(address, value); });
			}
		}

		for (auto& tpdo : transmitPdos_)
		{
			tpdo.processMessage(
				message, [](Address address) { return read(address); },
				std::forward<MessageCallback>(cb));
		}

		(Protocols::template processMessage<CanopenDevice, MessageCallback>(
			 message, std::forward<MessageCallback>(cb)),
		 ...);
	}
}

template<typename OD, typename... Protocols>
template<typename MessageCallback>
void
CanopenDevice<OD, Protocols...>::update(MessageCallback&& cb)
{
	const auto isInSync = isInSyncWindow();
	justLeftSyncWindow_ = (wasInSyncWindow_ && !isInSync);
	wasInSyncWindow_ = isInSync;
	if (syncPeriod_.count() != 0 && lastSyncTime_.time_since_epoch().count() != 0)
	{
		const auto now = modm::PreciseClock::now();
		const auto timeSinceSync = now - lastSyncTime_;
		if (timeSinceSync > syncPeriod_ + 1ms && !missedSync_)
		{
			MODM_LOG_DEBUG << "Missed Sync by more than 1 ms!" << modm::endl;
			setError(EMCYError::GenericCommunicationError);
			missedSync_ = true;
		}
	}

	if (emcyDue_ && (lastEmcyTime_.time_since_epoch().count() == 0 ||
					 modm::PreciseClock::now() - lastEmcyTime_ > emcyInhibitTime_))
	{
		sendEMCY(std::forward<MessageCallback>(cb));
	}
	Heartbeat<CanopenDevice>::update(std::forward<MessageCallback>(cb));
	if (state_ == NMTState::Operational)
	{
		for (auto& tpdo : transmitPdos_)
		{
			if (tpdo.isActive())
			{
				auto message = tpdo.nextMessage(isInSyncWindow(),
												[](Address address) { return read(address); });
				if (message) { std::forward<MessageCallback>(cb)(*message); }
			}
		}
		for (auto& rpdo : receivePdos_)
		{
			if (rpdo.isActive() && rpdo.getTransmitMode().isOnSync())
			{
				rpdo.template update<CanopenDevice>(wasInSyncWindow_);
			}
		}
		(Protocols::template update<CanopenDevice, MessageCallback>(
			 std::forward<MessageCallback>(cb)),
		 ...);
	}
}

template<typename OD, typename... Protocols>
void
CanopenDevice<OD, Protocols...>::setValueChanged(Address address)
{
	for (auto& tpdo : transmitPdos_)
	{
		if (tpdo.isActive())
		{
			for (uint_fast8_t i = 0; i < tpdo.mappingCount(); ++i)
			{
				if (tpdo.mapping(i).address == address)
				{
					tpdo.setValueUpdated();
					break;
				}
			}
		}
	}
}

template<typename OD, typename... Protocols>
void
CanopenDevice<OD, Protocols...>::setNodeId(uint8_t id)
{
	nodeId_ = id & 0x7f;
	static_assert(transmitPdos_.size() <= 64);
	static_assert(receivePdos_.size() <= 64);
	// TODO remove?
	for (size_t i = 0; i < transmitPdos_.size(); ++i)
	{
		transmitPdos_[i].setCanId((0x100 * (i + 1) + 0x80) | nodeId_);
	}
	for (size_t i = 0; i < receivePdos_.size(); ++i)
	{
		receivePdos_[i].setCanId(0x100 * (i + 2) | nodeId_);
	}
	// TODO remove?
	emcyCobId_ = nodeId_ + 0x80u;
	SdoServer<CanopenDevice>::setNodeId(id);
}

template<typename OD, typename... Protocols>
uint8_t
CanopenDevice<OD, Protocols...>::nodeId()
{
	return nodeId_;
}

template<typename OD, typename... Protocols>
NMTState
CanopenDevice<OD, Protocols...>::nmtState()
{
	return state_;
}

template<typename OD, typename... Protocols>
bool
CanopenDevice<OD, Protocols...>::isInSyncWindow()
{
	if (lastSyncTime_.time_since_epoch().count() == 0) return false;
	return (modm::PreciseClock::now() - lastSyncTime_) < syncWindowDuration_;
}

template<typename OD, typename... Protocols>
uint8_t
CanopenDevice<OD, Protocols...>::syncCounter()
{
	return lastSyncCounter_;
}

template<typename OD, typename... Protocols>
EMCYError
CanopenDevice<OD, Protocols...>::getEMCYError()
{
	return emcy_;
}

template<typename OD, typename... Protocols>
void
CanopenDevice<OD, Protocols...>::setError(EMCYError emcy)
{
	emcyDue_ = true;
	emcy_ = emcy;
	if (((uint16_t)emcy & 0xFF00) == (uint16_t)EMCYError::GenericCommunicationError)
	{
		// Handle communication error
		// TODO implement 0x1029.1
		if (state_ == NMTState::Operational)
		{
			MODM_LOG_ERROR << "Communication Error! Going into PreOperational!" << modm::endl;
			state_ = NMTState::PreOperational;
		}
	}
}

template<typename OD, typename... Protocols>
uint8_t&
CanopenDevice<OD, Protocols...>::getErrorRegister()
{
	return errorReg_;
}

template<typename OD, typename... Protocols>
std::array<uint8_t, 5>&
CanopenDevice<OD, Protocols...>::getManufacturerError()
{
	return manufacturerError_;
}

template<typename OD, typename... Protocols>
constexpr auto
CanopenDevice<OD, Protocols...>::registerHandlers() -> HandlerMap<OD>
{
	HandlerMap<OD> handlers;
	handlers.template setReadHandler<Address{0x1000, 0}>(+[]() { return deviceId_.deviceType_; });
	handlers.template setReadHandler<Address{0x1001, 0}>(+[]() { return errorReg_; });
	handlers.template setReadHandler<Address{0x1003, 0}>(+[]() {
		if (emcy_ != EMCYError::NoError)
			return (uint32_t)1;
		else
			return (uint32_t)0;
	});
	handlers.template setReadHandler<Address{0x1003, 1}>(+[]() { return (uint32_t)emcy_; });

	handlers.template setReadHandler<Address{0x1005, 0}>(+[]() {
		const bool extended = ((syncCobId_ & 0x1FFFF800) != 0);
		return (syncCobId_ & 0x1FFFFFFFu) | (extended ? 0x20000000u : 0x00000000u);
	});
	handlers.template setWriteHandler<Address{0x1005, 0}>(+[](uint32_t val) {
		// Sync producing is unsupported
		if ((val & 0x40000000) != 0) return SdoErrorCode::UnsupportedAccess;

		// Check if extended flag is set correctly
		uint32_t newId = (val & 0x1FFFFFFFu);
		if ((newId & 0x1FFFF800) != 0 && (val & 0x20000000u) == 0)
			return SdoErrorCode::InvalidValue;
		syncCobId_ = newId;
		return SdoErrorCode::NoError;
	});

	handlers.template setReadHandler<Address{0x1006, 0}>(
		+[]() { return (uint32_t)syncPeriod_.count(); });
	handlers.template setWriteHandler<Address{0x1006, 0}>(+[](uint32_t val) {
		syncPeriod_ = std::chrono::microseconds(val);
		return SdoErrorCode::NoError;
	});

	handlers.template setReadHandler<Address{0x1007, 0}>(
		+[]() { return (uint32_t)syncWindowDuration_.count(); });
	handlers.template setWriteHandler<Address{0x1007, 0}>(+[](uint32_t val) {
		syncWindowDuration_ = std::chrono::microseconds(val);
		return SdoErrorCode::NoError;
	});

	handlers.template setReadHandler<Address{0x1014, 0}>(+[]() {
		uint32_t out = emcyCobId_ & 0x1F'FF'FF'FF;
		if (emcyEnabled_) out |= 0x80'00'00'00;
		if (emcyCobId_ & 0x1F'FF'F8'00) out |= 0x2F'FF'FF'FF;
		return out;
	});
	handlers.template setWriteHandler<Address{0x1014, 0}>(+[](uint32_t val) {
		if (val & 0x40'00'00'00) return SdoErrorCode::InvalidValue;
		if ((bool)(val & 0x1F'FF'F8'00) != (bool)(val & 0x20'00'00'00))
			return SdoErrorCode::InvalidValue;
		emcyEnabled_ = (bool)(val & 0x80'00'00'00);
		emcyCobId_ = val & 0x1F'FF'FF'FF;
		return SdoErrorCode::NoError;
	});

	handlers.template setReadHandler<Address{0x1015, 0}>(
		+[]() { return (uint16_t)(emcyInhibitTime_.count() / 100); });
	handlers.template setWriteHandler<Address{0x1015, 0}>(+[](uint16_t val) {
		emcyInhibitTime_ = std::chrono::microseconds(100 * val);
		return SdoErrorCode::NoError;
	});

	handlers.template setReadHandler<Address{0x1018, 0}>(+[]() { return (uint8_t)4; });
	handlers.template setReadHandler<Address{0x1018, 1}>(+[]() { return deviceId_.vendorId_; });
	handlers.template setReadHandler<Address{0x1018, 2}>(+[]() { return deviceId_.productCode_; });
	handlers.template setReadHandler<Address{0x1018, 3}>(+[]() { return deviceId_.revisionId_; });
	handlers.template setReadHandler<Address{0x1018, 4}>(+[]() { return deviceId_.serialNumber_; });

	handlers.template setReadHandler<Address{0x1019, 0}>(+[]() { return syncCounterOverflow_; });
	handlers.template setWriteHandler<Address{0x1019, 0}>(+[](uint8_t val) {
		if (val == 1 || val > 240) return SdoErrorCode::InvalidValue;
		syncCounterOverflow_ = val;
		return SdoErrorCode::NoError;
	});

	Heartbeat<CanopenDevice>{}.registerHandlers(handlers);
	ReceivePdoConfigurator<CanopenDevice>{}.registerHandlers(handlers);
	TransmitPdoConfigurator<CanopenDevice>{}.registerHandlers(handlers);
	SdoServer<CanopenDevice>{}.registerHandlers(handlers);
	(Protocols{}.registerHandlers(handlers), ...);

	return handlers;
}

template<typename OD, typename... Protocols>
constexpr auto
CanopenDevice<OD, Protocols...>::constructHandlerMap() -> HandlerMap<OD>
{
	constexpr HandlerMap<OD> handlers = registerHandlers();
	detail::missing_read_handler<findMissingReadHandler(handlers)>();
	detail::missing_write_handler<findMissingWriteHandler(handlers)>();
	return handlers;
}

template<typename OD, typename... Protocols>
void
CanopenDevice<OD, Protocols...>::setReceivePdoActive(uint8_t index, bool active)
{
	receivePdos_[index].setActive(active);
}

template<typename OD, typename... Protocols>
void
CanopenDevice<OD, Protocols...>::setTransmitPdoActive(uint8_t index, bool active)
{
	transmitPdos_[index].setActive(active);
}

template<typename OD, typename... Protocols>
void
CanopenDevice<OD, Protocols...>::setReceivePdo(uint8_t index, ReceivePdo_t rpdo)
{
	receivePdos_[index] = rpdo;
	receivePdos_[index].setCanId(rpdoCanId(index));
}

template<typename OD, typename... Protocols>
void
CanopenDevice<OD, Protocols...>::setTransmitPdo(uint8_t index, TransmitPdo_t tpdo)
{
	transmitPdos_[index] = tpdo;
	transmitPdos_[index].setCanId(tpdoCanId(index));
}

template<typename OD, typename... Protocols>
uint32_t
CanopenDevice<OD, Protocols...>::tpdoCanId(uint8_t index)
{
	return (0x100 * (index + 1) + 0x80) + nodeId_;
}
template<typename OD, typename... Protocols>
uint32_t
CanopenDevice<OD, Protocols...>::rpdoCanId(uint8_t index)
{
	return (0x100 * (index + 1) + 0x100) + nodeId_;
}

}  // namespace modm_canopen
