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
			valueToBytes(*ptr, message.data + index);
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
	sync_ = true;
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
TransmitPdo<OD>::nextMessage(Callback &&cb)
{
	const bool send = (transmitMode_ == TransmitMode::OnSync && sync_) || sendOnEvent_.send();

	if (send)
	{
		return getMessage(std::forward<Callback>(cb));
	} else
	{
		return std::nullopt;
	}
}

template<typename OD>
void
TransmitPdo<OD>::setTransmitMode(TransmitMode mode)
{
	transmitMode_ = mode;
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

}  // namespace modm_canopen
