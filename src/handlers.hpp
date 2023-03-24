#pragma once
#include <cstdint>
#include <variant>
#include "float_types.hpp"
#include "sdo_error.hpp"

namespace modm_canopen
{

template<typename T>
using ReadFunction = T (*)();

template<typename T>
using WriteFunction = SdoErrorCode (*)(T);

using ReadHandler = std::variant<std::monostate, ReadFunction<uint8_t>, ReadFunction<uint16_t>,
								 ReadFunction<uint32_t>, ReadFunction<uint64_t>,
								 ReadFunction<int8_t>, ReadFunction<int16_t>, ReadFunction<int32_t>,
								 ReadFunction<int64_t>, ReadFunction<float32_t>>;

using WriteHandler =
	std::variant<std::monostate, WriteFunction<uint8_t>, WriteFunction<uint16_t>,
				 WriteFunction<uint32_t>, WriteFunction<uint64_t>, WriteFunction<int8_t>,
				 WriteFunction<int16_t>, WriteFunction<int32_t>, WriteFunction<int64_t>,
				 WriteFunction<float32_t>>;

namespace detail
{

template<Address address>
struct missing_read_handler
{
	static_assert(address == Address{}, "Read handler not registered for at least one object");
};

template<Address address>
struct missing_write_handler
{
	static_assert(address == Address{}, "Write handler not registered for at least one object");
};

}  // namespace detail

inline Value
callReadHandler(ReadHandler h)
{
	static_assert(Value(std::monostate{}).index() == size_t(DataType::Empty));

	switch (DataType(h.index()))
	{
		case DataType::Empty:
			return Value{};
		case DataType::UInt8:
			return Value(std::get<ReadFunction<uint8_t>>(h)());
		case DataType::UInt16:
			return Value(std::get<ReadFunction<uint16_t>>(h)());
		case DataType::UInt32:
			return Value(std::get<ReadFunction<uint32_t>>(h)());
		case DataType::UInt64:
			return Value(std::get<ReadFunction<uint64_t>>(h)());
		case DataType::Int8:
			return Value(std::get<ReadFunction<int8_t>>(h)());
		case DataType::Int16:
			return Value(std::get<ReadFunction<int16_t>>(h)());
		case DataType::Int32:
			return Value(std::get<ReadFunction<int32_t>>(h)());
		case DataType::Int64:
			return Value(std::get<ReadFunction<int64_t>>(h)());
		case DataType::Real32:
			return Value(std::get<ReadFunction<float32_t>>(h)());
	}
	return Value{};
}

inline SdoErrorCode
callWriteHandler(WriteHandler h, Value value)
{
	switch (DataType(h.index()))
	{
		case DataType::UInt8:
			return std::get<WriteFunction<uint8_t>>(h)(*std::get_if<uint8_t>(&value));
		case DataType::UInt16:
			return std::get<WriteFunction<uint16_t>>(h)(*std::get_if<uint16_t>(&value));
		case DataType::UInt32:
			return std::get<WriteFunction<uint32_t>>(h)(*std::get_if<uint32_t>(&value));
		case DataType::UInt64:
			return std::get<WriteFunction<uint64_t>>(h)(*std::get_if<uint64_t>(&value));
		case DataType::Int8:
			return std::get<WriteFunction<int8_t>>(h)(*std::get_if<int8_t>(&value));
		case DataType::Int16:
			return std::get<WriteFunction<int16_t>>(h)(*std::get_if<int16_t>(&value));
		case DataType::Int32:
			return std::get<WriteFunction<int32_t>>(h)(*std::get_if<int32_t>(&value));
		case DataType::Int64:
			return std::get<WriteFunction<int64_t>>(h)(*std::get_if<int64_t>(&value));
		case DataType::Real32:
			return std::get<WriteFunction<float32_t>>(h)(*std::get_if<float32_t>(&value));
		case DataType::Empty:
			break;
	}
	return SdoErrorCode::GeneralError;
}

static_assert(ReadHandler(std::monostate{}).index() == size_t(DataType::Empty));
static_assert(ReadHandler(ReadFunction<uint8_t>{}).index() == size_t(DataType::UInt8));
static_assert(ReadHandler(ReadFunction<uint16_t>{}).index() == size_t(DataType::UInt16));
static_assert(ReadHandler(ReadFunction<uint32_t>{}).index() == size_t(DataType::UInt32));
static_assert(ReadHandler(ReadFunction<uint64_t>{}).index() == size_t(DataType::UInt64));
static_assert(ReadHandler(ReadFunction<int8_t>{}).index() == size_t(DataType::Int8));
static_assert(ReadHandler(ReadFunction<int16_t>{}).index() == size_t(DataType::Int16));
static_assert(ReadHandler(ReadFunction<int32_t>{}).index() == size_t(DataType::Int32));
static_assert(ReadHandler(ReadFunction<int64_t>{}).index() == size_t(DataType::Int64));
static_assert(ReadHandler(ReadFunction<float32_t>{}).index() == size_t(DataType::Real32));

static_assert(WriteHandler(std::monostate{}).index() == size_t(DataType::Empty));
static_assert(WriteHandler(WriteFunction<uint8_t>{}).index() == size_t(DataType::UInt8));
static_assert(WriteHandler(WriteFunction<uint16_t>{}).index() == size_t(DataType::UInt16));
static_assert(WriteHandler(WriteFunction<uint32_t>{}).index() == size_t(DataType::UInt32));
static_assert(WriteHandler(WriteFunction<uint64_t>{}).index() == size_t(DataType::UInt64));
static_assert(WriteHandler(WriteFunction<int8_t>{}).index() == size_t(DataType::Int8));
static_assert(WriteHandler(WriteFunction<int16_t>{}).index() == size_t(DataType::Int16));
static_assert(WriteHandler(WriteFunction<int32_t>{}).index() == size_t(DataType::Int32));
static_assert(WriteHandler(WriteFunction<int64_t>{}).index() == size_t(DataType::Int64));
static_assert(ReadHandler(ReadFunction<float32_t>{}).index() == size_t(DataType::Real32));

}  // namespace modm_canopen