#ifndef CANOPEN_PDO_COMMON_HPP
#define CANOPEN_PDO_COMMON_HPP

#include "object_dictionary.hpp"
#include "sdo_error.hpp"

namespace modm_canopen {

struct PdoMapping {
  Address address;
  uint8_t bitLength;

  uint32_t inline encode() {
    return uint32_t(bitLength) | uint32_t(address.subindex << 8) |
           uint32_t(address.index << 16);
  }

  static inline PdoMapping decode(uint32_t value) {
    return PdoMapping{
        .address = {.index = uint16_t((value & 0xFFFF'0000) >> 16),
                    .subindex = uint8_t((value & 0xFF00) >> 8)},
        .bitLength = uint8_t(value & 0xFF)};
  }
};

template <typename OD> class PdoObject {
protected:
  static constexpr std::size_t MaxMappingCount{8};

  bool active_{false};
  uint32_t canId_{};
  uint_fast8_t mappingCount_{};
  std::array<PdoMapping, MaxMappingCount> mappings_{};
  std::array<DataType, MaxMappingCount> mappingTypes_{};

public:
  void setCanId(uint32_t canId);

  SdoErrorCode setActive();
  void setInactive();
  bool isActive() const;

  SdoErrorCode setMappingCount(uint_fast8_t count);
  uint_fast8_t mappingCount() const;

  SdoErrorCode setMapping(uint_fast8_t index, PdoMapping mapping);
  PdoMapping mapping(uint_fast8_t index) const;

  uint32_t cobId() const { return active_ ? canId_ : (canId_ | 0x8000'0000); }
  uint32_t canId() const { return canId_; }

protected:
  virtual SdoErrorCode validateMapping(PdoMapping mapping) = 0;
  SdoErrorCode validateMappings();
};

} // namespace modm_canopen

#include "pdo_common_impl.hpp"

#endif // CANOPEN_PDO_COMMON_HPP
