#ifndef CANOPEN_CANOPEN_DEVICE_HPP
#error "Do not include this file directly, use canopen_device.hpp instead"
#endif

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
CanopenDevice<OD, Protocols...>::write(Address address, std::span<const uint8_t> data, int8_t size)
	-> SdoErrorCode
{
	auto entry = OD::map.lookup(address);
	if (!entry) { return SdoErrorCode::ObjectDoesNotExist; }
	if (!entry->isWritable()) { return SdoErrorCode::WriteOfReadOnlyObject; }

	const auto objectSize = getDataTypeSize(entry->dataType);
	const bool sizeIsValid =
		(objectSize <= data.size()) && ((size == -1) || (size == int8_t(objectSize)));
	if (!sizeIsValid) { return SdoErrorCode::UnsupportedAccess; }

	auto handler = accessHandlers.lookupWriteHandler(address);
	if (handler)
	{
		const Value value = valueFromBytes(entry->dataType, data.data());
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
			return SdoErrorCode::ObjectDoesNotExist;
		} else if (entry->isReadable())
		{
			return SdoErrorCode::ReadOfWriteOnlyObject;
		}
	}
	return SdoErrorCode::UnsupportedAccess;
}

template<typename OD, typename... Protocols>
template<typename MessageCallback>
void
CanopenDevice<OD, Protocols...>::processMessage(const modm::can::Message& message,
												MessageCallback&& cb)
{
	for (auto& rpdo : receivePdos_)
	{
		rpdo.processMessage(message, [](Address address, Value value) { write(address, value); });
	}
	if ((message.identifier & 0x7f) == nodeId_)
	{
		SdoServer<CanopenDevice>::processMessage(message, std::forward<MessageCallback>(cb));
	}
	(Protocols::template processMessage<CanopenDevice<OD, Protocols...>, MessageCallback>(
		 message, std::forward<MessageCallback>(cb)),
	 ...);
}

template<typename OD, typename... Protocols>
template<typename MessageCallback>
void
CanopenDevice<OD, Protocols...>::update(MessageCallback&& cb)
{
	for (auto& tpdo : transmitPdos_)
	{
		if (tpdo.isActive())
		{
			auto message = tpdo.nextMessage([](Address address) { return read(address); });
			if (message) { std::forward<MessageCallback>(cb)(*message); }
		}
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
	SdoServer<CanopenDevice>::setNodeId(id);
}

template<typename OD, typename... Protocols>
uint8_t
CanopenDevice<OD, Protocols...>::nodeId()
{
	return nodeId_;
}

template<typename OD, typename... Protocols>
constexpr auto
CanopenDevice<OD, Protocols...>::registerHandlers() -> HandlerMap<OD>
{
	HandlerMap<OD> handlers;
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
