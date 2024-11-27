#ifndef CANOPEN_TRANSMIT_PDO_HPP
#error "Do not include this file directly, include transmit_pdo.hpp instead!"
#endif

namespace modm_canopen
{

template<typename OD>
template<typename Callback>
std::optional<modm::can::Message>
TransmitPdo<OD>::getMessage(Callback &&cb)
{
	rtr_ = false;
	syncCount_ = 0;
	hasReceivedSync_ = false;
	sendOnEvent_.updated_ = false;

	modm::can::Message message{PdoObject<OD>::canId_};
	message.setExtended(false);

	if (PdoObject<OD>::active_ && PdoObject<OD>::mappingCount_ > 0)
	{
		std::size_t index = 0;
		for (uint_fast8_t i = 0; i < PdoObject<OD>::mappingCount_; ++i)
		{
			const auto address = PdoObject<OD>::mappings_[i].address;
			const auto size = PdoObject<OD>::mappings_[i].bitLength / 8;
			const auto value = std::forward<Callback>(cb)(address);
			const auto *ptr = std::get_if<Value>(&value);
			if (!ptr) return std::nullopt;
			valueToBytes(*ptr, std::span<uint8_t>(message.data + index, message.capacity - index));
			index += size;
		}
		message.setLength(index);
	}
	return message;
}

template<typename OD>
void
TransmitPdo<OD>::sync()
{
	hasReceivedSync_ = true;
	syncCount_++;
}

template<typename OD>
void
TransmitPdo<OD>::setValueUpdated()
{
	sendOnEvent_.updated_ = true;
}

template<typename OD>
template<typename Callback>
std::optional<modm::can::Message>
TransmitPdo<OD>::nextMessage(bool inSync, Callback &&cb)
{
	const bool sendOnEvent = sendOnEvent_.send();

	const bool sendOnSyncEventDriven =
		PdoObject<OD>::getTransmitMode().value == 0 && sendOnEvent && hasReceivedSync_;
	const bool sendOnSyncCounter = PdoObject<OD>::getTransmitMode().isOnSync() &&
								   PdoObject<OD>::getTransmitMode().value != 0 &&
								   hasReceivedSync_ && inSync &&
								   syncCount_ >= PdoObject<OD>::getTransmitMode().value;
	const bool sendAsync = PdoObject<OD>::getTransmitMode().isAsync() && sendOnEvent;
	const bool sendRTRSync =
		(rtr_ && PdoObject<OD>::getTransmitMode().value == 0xFC && inSync && syncCount_ != 0);

	const bool send = sendOnSyncCounter || sendOnSyncEventDriven || sendAsync || sendRTRSync;

	if (send)
	{
		return getMessage(std::forward<Callback>(cb));
	} else
	{
		return std::nullopt;
	}
}

template<typename OD>
bool
TransmitPdo<OD>::setTransmitMode(uint8_t mode)
{
	if (mode > 0xF0 && mode < 0xFC) return false;
	PdoObject<OD>::mode_.value = mode;
	return true;
}

template<typename OD>
SdoErrorCode
TransmitPdo<OD>::setEventTimeout(uint16_t milliseconds)
{
	sendOnEvent_.eventTimeout_ = std::chrono::milliseconds(milliseconds);
	return SdoErrorCode::NoError;
}

template<typename OD>
SdoErrorCode
TransmitPdo<OD>::setInhibitTime(uint16_t inhibitTime_100us)
{
	sendOnEvent_.inhibitTime_ = std::chrono::microseconds(inhibitTime_100us * 100);
	return SdoErrorCode::NoError;
}

template<typename OD>
uint16_t
TransmitPdo<OD>::eventTimeout() const
{
	return std::chrono::duration_cast<modm::Duration>(sendOnEvent_.eventTimeout_).count();
}

template<typename OD>
uint16_t
TransmitPdo<OD>::inhibitTime() const
{
	return sendOnEvent_.inhibitTime_.count() / 100;
}

template<typename OD>
SdoErrorCode
TransmitPdo<OD>::validateMapping(PdoMapping mapping)
{
	const auto entry = OD::map.lookup(mapping.address);
	if (!entry) { return SdoErrorCode::ObjectDoesNotExist; }
	if (!entry->isTransmitPdoMappable()) { return SdoErrorCode::PdoMappingError; }
	if (getDataTypeSize(entry->dataType) * 8 != mapping.bitLength)
	{
		return SdoErrorCode::PdoMappingError;
	}
	return SdoErrorCode::NoError;
}

template<typename OD>
template<typename ReadCallback, typename MessageCallback>
void
TransmitPdo<OD>::processMessage(const modm::can::Message &message, ReadCallback &&read,
								MessageCallback &&cb)
{
	if (message.getIdentifier() == PdoObject<OD>::cobId() && message.isRemoteTransmitRequest())
	{
		rtr_ = true;
	}
	if (rtr_ && PdoObject<OD>::getTransmitMode().value == 0xFD)
	{
		auto newMsg = getMessage(std::forward<ReadCallback>(read));
		if (newMsg) cb(*newMsg);
	}
}

}  // namespace modm_canopen
