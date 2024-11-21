#ifndef CANOPEN_RECEIVE_PDO_HPP
#error "Do not include this file directly, include receive_pdo.hpp instead!"
#endif
#include "emcy_error.hpp"

namespace modm_canopen
{

template<typename OD>
template<typename Callback>
void
ReceivePdo<OD>::processMessage(const modm::can::Message &message, Callback &&cb)
{
	if (message.identifier != PdoObject<OD>::canId_) { return; }
	if (PdoObject<OD>::active_ && PdoObject<OD>::mappingCount_ > 0)
	{
		std::size_t totalDataSize = 0;
		for (uint_fast8_t i = 0; i < PdoObject<OD>::mappingCount_; ++i)
		{
			totalDataSize += PdoObject<OD>::mappings_[i].bitLength / 8;
		}
		if (totalDataSize > message.getLength())
		{
			// TODO set EMCY
			return;
		}
		std::size_t index = 0;
		for (uint_fast8_t i = 0; i < PdoObject<OD>::mappingCount_; ++i)
		{
			const auto address = PdoObject<OD>::mappings_[i].address;
			const auto size = PdoObject<OD>::mappings_[i].bitLength / 8;
			const auto value = valueFromBytes(
				PdoObject<OD>::mappingTypes_[i],
				std::span<const uint8_t>(message.data + index, message.capacity - index));
			std::forward<Callback>(cb)(address, value);
			index += size;
		}
		received_ = true;
	}
}

template<typename OD>
SdoErrorCode
ReceivePdo<OD>::validateMapping(PdoMapping mapping)
{
	const auto entry = OD::map.lookup(mapping.address);
	if (!entry) { return SdoErrorCode::ObjectDoesNotExist; }
	if (!entry->isReceivePdoMappable()) { return SdoErrorCode::PdoMappingError; }
	if (getDataTypeSize(entry->dataType) * 8 != mapping.bitLength)
	{
		return SdoErrorCode::PdoMappingError;
	}
	return SdoErrorCode::NoError;
}

template<typename OD>
bool
ReceivePdo<OD>::setTransmitMode(uint8_t mode)
{
	if (mode > 0xF0 && mode < 0xFE) return false;
	passedSyncs_ = 0;
	received_ = false;
	PdoObject<OD>::mode_.value = mode;
	return true;
}

template<typename OD>
template<typename Device>
void
ReceivePdo<OD>::update(bool wasJustInSync)
{
	if (wasJustInSync) { passedSyncs_++; }
	if (received_ && passedSyncs_ != PdoObject<OD>::mode_.value)
	{
		// Missed RPDO
		if (PdoObject<OD>::mode_.value > 0 && PdoObject<OD>::mode_.isOnSync())
		{
			Device::setError(EMCYError::RPDOTimeout);
		}
	}
	if (passedSyncs_ >= PdoObject<OD>::mode_.value) { passedSyncs_ = 0; }
	received_ = false;
}
}  // namespace modm_canopen
