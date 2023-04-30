#ifndef CANOPEN_RECEIVE_PDO_HPP
#error "Do not include this file directly, include receive_pdo.hpp instead!"
#endif

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
		if (totalDataSize > message.getLength()) { return; }
		std::size_t index = 0;
		for (uint_fast8_t i = 0; i < PdoObject<OD>::mappingCount_; ++i)
		{
			const auto address = PdoObject<OD>::mappings_[i].address;
			const auto size = PdoObject<OD>::mappings_[i].bitLength / 8;
			const auto value =
				valueFromBytes(PdoObject<OD>::mappingTypes_[i], message.data + index);
			std::forward<Callback>(cb)(address, value);
			index += size;
		}
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
}  // namespace modm_canopen
