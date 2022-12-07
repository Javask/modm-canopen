#ifndef CANOPEN_PDO_COMMON_HPP
#error "Do not include this file directly, include pdo_common.hpp instead!"
#endif

namespace modm_canopen
{

template<typename OD>
void PdoObject<OD>::setCanId(uint32_t canId)
{
    canId_ = canId;
}

template<typename OD>
SdoErrorCode PdoObject<OD>::setActive()
{
    if(const auto error = validateMappings(); error != SdoErrorCode::NoError) {
        return error;
    }

    active_ = true;
    return SdoErrorCode::NoError;
}

template<typename OD>
void PdoObject<OD>::setInactive()
{
    active_ = false;
}

template<typename OD>
bool PdoObject<OD>::isActive() const
{
    return active_;
}


template<typename OD>
SdoErrorCode PdoObject<OD>::setMappingCount(uint_fast8_t count)
{
    if (active_ || count > MaxMappingCount) {
        return SdoErrorCode::UnsupportedAccess;
    }

    unsigned totalSize = 0;
    for (uint_fast8_t i = 0; i < count; ++i) {
        const auto error = validateMapping(mappings_[i]);
        if (error != SdoErrorCode::NoError) {
            return error;
        }
        const auto entry = OD::map.lookup(mappings_[i].address);
        mappingTypes_[i] = entry->dataType;
        totalSize += mappings_[i].bitLength;
    }
    if (totalSize > 8*8) {
        return SdoErrorCode::MappingsExceedPdoLength;
    }

    mappingCount_ = count;
    return SdoErrorCode::NoError;
}

template<typename OD>
uint_fast8_t PdoObject<OD>::mappingCount() const
{
    return mappingCount_;
}

template<typename OD>
SdoErrorCode PdoObject<OD>::setMapping(uint_fast8_t index, PdoMapping mapping)
{
    const auto error = validateMapping(mapping);
    if (error == SdoErrorCode::NoError) {
        mappings_[index] = mapping;
    }
    return error;
}

template<typename OD>
PdoMapping PdoObject<OD>::mapping(uint_fast8_t index) const
{
    return mappings_[index];
}

template<typename OD>
SdoErrorCode PdoObject<OD>::validateMappings()
{
    return SdoErrorCode::NoError;
}

template<typename OD>
SdoErrorCode PdoObject<OD>::validateMapping(PdoMapping mapping)
{
    const auto entry = OD::map.lookup(mapping.address);
    if (!entry) {
        return SdoErrorCode::ObjectDoesNotExist;
    }
    if (!entry->isReceivePdoMappable()) {
        return SdoErrorCode::PdoMappingError;
    }
    if (getDataTypeSize(entry->dataType)*8 != mapping.bitLength) {
        return SdoErrorCode::PdoMappingError;
    }
    return SdoErrorCode::NoError;
}

}
