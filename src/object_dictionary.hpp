#ifndef CANOPEN_OBJECT_DICTIONARY_HPP
#define CANOPEN_OBJECT_DICTIONARY_HPP

#include "object_dictionary_common.hpp"
#include "constexpr_map.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <numeric>
#include <optional>
#include <utility>
#include <span>

#include "sdo_error.hpp"

namespace modm_canopen
{

template<typename Map>
constexpr std::size_t
readableEntryCount()
{
	const auto isReadable = [](const std::pair<Address, Entry>& elem) {
		return elem.second.isReadable() ? 1u : 0u;
	};
	return std::transform_reduce(Map::map.begin(), Map::map.end(), 0u, std::plus<>{}, isReadable);
}

template<typename Map>
constexpr std::size_t
writableEntryCount()
{
	const auto isWritable = [](const std::pair<Address, Entry>& elem) {
		return elem.second.isWritable() ? 1u : 0u;
	};
	return std::transform_reduce(Map::map.begin(), Map::map.end(), 0u, std::plus<>{}, isWritable);
}

inline size_t
getDataTypeSize(DataType type)
{
	switch (type)
	{
		case DataType::Empty:
			return 0;
		case DataType::UInt8:
			return 1;
		case DataType::UInt16:
			return 2;
		case DataType::UInt32:
			return 4;
		case DataType::UInt64:
			return 8;
		case DataType::Int8:
			return 1;
		case DataType::Int16:
			return 2;
		case DataType::Int32:
			return 4;
		case DataType::Int64:
			return 8;
		case DataType::Real32:
			return 4;
	}
	return 0;
}

inline size_t
getValueSize(const Value& value)
{
	return getDataTypeSize(DataType(value.index()));
}

inline bool
typeSupportsExpediteTransfer(DataType type)
{
	return (type != DataType::Empty) && (type != DataType::UInt64) && (type != DataType::Int64);
}

inline bool
valueSupportsExpediteTransfer(const Value& value)
{
	const auto type = static_cast<DataType>(value.index());
	return typeSupportsExpediteTransfer(type);
}

inline Value
valueFromBytes(DataType type, std::span<const uint8_t> data)
{
	switch (type)
	{
		case DataType::UInt8:
			if(data.size() < sizeof(uint8_t)) return Value();
			return Value(uint8_t(data[0]));
		case DataType::UInt16:
			if(data.size() < sizeof(uint16_t)) return Value();
			return Value(uint16_t(data[0] | (data[1] << 8)));
		case DataType::UInt32:
			if(data.size() < sizeof(uint32_t)) return Value();
			return Value(uint32_t(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24)));
		case DataType::UInt64:
			if(data.size() < sizeof(uint64_t)) return Value();
			return Value(uint64_t(data[0]) | (uint64_t(data[1]) << 8) | (uint64_t(data[2]) << 16) |
						 (uint64_t(data[3]) << 24) | (uint64_t(data[4]) << 32) |
						 (uint64_t(data[5]) << 40) | (uint64_t(data[6]) << 48) |
						 (uint64_t(data[7]) << 56));
		case DataType::Int8:
			if(data.size() < sizeof(int8_t)) return Value();
			return Value(int8_t(data[0]));
		case DataType::Int16:
			if(data.size() < sizeof(int16_t)) return Value();
			return Value(int16_t(data[0] | (data[1] << 8)));
		case DataType::Int32:
			if(data.size() < sizeof(int32_t)) return Value();
			return Value(int32_t(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24)));
		case DataType::Int64:
			if(data.size() < sizeof(int64_t)) return Value();
			return Value(int64_t(data[0]) | (int64_t(data[1]) << 8) | (int64_t(data[2]) << 16) |
						 (int64_t(data[3]) << 24) | (int64_t(data[4]) << 32) |
						 (int64_t(data[5]) << 40) | (int64_t(data[6]) << 48) |
						 (int64_t(data[7]) << 56));
		case DataType::Real32: {

			if(data.size() < sizeof(float32_t)) return Value();
			float32_t temp;
			memcpy(&temp, data.data(), sizeof(float32_t));
			// TODO replace with guaranteed 32bit float type if that exists
			return Value(temp);
		}
		case DataType::Empty:
			return Value{};
	}
	return Value{};
}

inline void
valueToBytes(const Value& val, std::span<uint8_t> out)
{
	if (std::holds_alternative<int8_t>(val))
	{
		if (out.size() >= sizeof(int8_t)) { out[0] = std::get<int8_t>(val); }
	} else if (std::holds_alternative<int16_t>(val))
	{
		if (out.size() >= sizeof(int16_t)) { *((int16_t*)out.data()) = std::get<int16_t>(val); }
	} else if (std::holds_alternative<int32_t>(val))
	{
		if (out.size() >= sizeof(int32_t)) { *((int32_t*)out.data()) = std::get<int32_t>(val); }
	} else if (std::holds_alternative<int64_t>(val))
	{
		if (out.size() >= sizeof(int64_t)) { *((int64_t*)out.data()) = std::get<int64_t>(val); }
	} else if (std::holds_alternative<uint8_t>(val))
	{
		if (out.size() >= sizeof(uint8_t)) { out[0] = std::get<uint8_t>(val); }
	} else if (std::holds_alternative<uint16_t>(val))
	{
		if (out.size() >= sizeof(uint16_t)) { *((uint16_t*)out.data()) = std::get<uint16_t>(val); }
	} else if (std::holds_alternative<uint32_t>(val))
	{
		if (out.size() >= sizeof(uint32_t)) { *((uint32_t*)out.data()) = std::get<uint32_t>(val); }
	} else if (std::holds_alternative<uint64_t>(val))
	{
		if (out.size() >= sizeof(uint64_t)) { *((uint64_t*)out.data()) = std::get<uint64_t>(val); }
	} else if (std::holds_alternative<float32_t>(val))
	{
		if (out.size() >= sizeof(float32_t))
		{
			*((float32_t*)out.data()) = std::get<float32_t>(val);
		}
	}
}
}  // namespace modm_canopen

#endif  // CANOPEN_OBJECT_DICTIONARY_HPP
