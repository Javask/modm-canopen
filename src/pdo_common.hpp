#ifndef CANOPEN_PDO_COMMON_HPP
#define CANOPEN_PDO_COMMON_HPP

#include "sdo_error.hpp"
#include "object_dictionary_common.hpp"

namespace modm_canopen
{

struct TransmitMode
{
	uint8_t value{};
	inline bool
	isOnSync() const
	{
		return value <= 0xF0;
	}
	inline bool
	isAsync() const
	{
		return value >= 0xFE;
	}
	// Only valid for TPDO
	inline bool
	isRTR(bool sync) const
	{
		return (sync && value == 0xFC) || value == 0xFD;
	}
};

struct PdoMapping
{
	Address address;
	uint8_t bitLength;

	uint32_t inline encode()
	{
		return uint32_t(bitLength) | uint32_t(address.subindex << 8) |
			   uint32_t(address.index << 16);
	}

	static inline PdoMapping
	decode(uint32_t value)
	{
		return PdoMapping{.address = {.index = uint16_t((value & 0xFFFF'0000) >> 16),
									  .subindex = uint8_t((value & 0xFF00) >> 8)},
						  .bitLength = uint8_t(value & 0xFF)};
	}
};

template<typename OD>
class PdoObject
{
protected:
	static constexpr std::size_t MaxMappingCount{8};

	bool active_{false};
	uint32_t canId_{};
	uint_fast8_t mappingCount_{};
	TransmitMode mode_{};
	std::array<PdoMapping, MaxMappingCount> mappings_{};
	std::array<DataType, MaxMappingCount> mappingTypes_{};

public:
	using ObjectDictionary = OD;

	void
	setCanId(uint32_t canId);

	SdoErrorCode
	setActive();
	void
	setInactive();
	bool
	isActive() const;

	SdoErrorCode
	setMappingCount(uint_fast8_t count);
	uint_fast8_t
	mappingCount() const;

	SdoErrorCode
	setMapping(uint_fast8_t index, PdoMapping mapping);
	PdoMapping
	mapping(uint_fast8_t index) const;

	const TransmitMode&
	getTransmitMode() const;

	uint32_t
	cobId() const
	{
		return active_ ? canId_ : (canId_ | 0x8000'0000);
	}
	uint32_t
	canId() const
	{
		return canId_;
	}

protected:
	virtual SdoErrorCode
	validateMapping(PdoMapping mapping) = 0;
	SdoErrorCode
	validateMappings();
};

}  // namespace modm_canopen

#include "pdo_common_impl.hpp"

#endif  // CANOPEN_PDO_COMMON_HPP
