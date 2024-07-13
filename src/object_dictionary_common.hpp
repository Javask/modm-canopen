#ifndef MODM_CANOPEN_OBJECT_DICTIONARY_COMMON_HPP
#define MODM_CANOPEN_OBJECT_DICTIONARY_COMMON_HPP

#include <cstdint>
#include <variant>
#include "float_types.hpp"

namespace modm_canopen
{

struct Address
{
	uint16_t index;
	uint8_t subindex;

	constexpr friend auto operator<=>(Address, Address) = default;
};

enum class DataType : uint8_t
{
	Empty,
	UInt8,
	UInt16,
	UInt32,
	UInt64,
	Int8,
	Int16,
	Int32,
	Int64,
	Real32
};

enum class AccessType : uint8_t
{
	ReadOnly,
	WriteOnly,
	ReadWrite,
	ReadWriteReadPdo,
	ReadWriteWritePdo
};

using Value = std::variant<std::monostate, uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t,
						   int32_t, int64_t, float32_t>;

struct Entry
{
	Address address;
	DataType dataType;
	AccessType accessType;
	bool pdoMapping;

	constexpr bool
	isReadable() const
	{
		return (accessType != AccessType::WriteOnly);
	}
	constexpr bool
	isWritable() const
	{
		return (accessType != AccessType::ReadOnly);
	}

	constexpr bool
	isReceivePdoMappable() const
	{
		return pdoMapping &&
			   (accessType == AccessType::WriteOnly || accessType == AccessType::ReadWriteWritePdo);
	}

	constexpr bool
	isTransmitPdoMappable() const
	{
		return pdoMapping &&
			   (accessType == AccessType::ReadOnly || accessType == AccessType::ReadWriteReadPdo);
	}
};

}  // namespace modm_canopen

#endif  // MODM_CANOPEN_OBJECT_DICTIONARY_COMMON_HPP
