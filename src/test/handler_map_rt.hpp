#ifndef CANOPEN_HANDLER_MAP_RT_HPP
#define CANOPEN_HANDLER_MAP_RT_HPP

#include <cstdint>
#include <variant>
#include <cassert>
#include <functional>
#include <map>
#include "float_types.hpp"
#include "sdo_error.hpp"
#include "handlers.hpp"

#include <modm/debug/logger.hpp>

namespace modm_canopen
{
template<typename T>
using ReadFunctionRT = std::function<T()>;

template<typename T>
using WriteFunctionRT = std::function<SdoErrorCode(T)>;

using ReadHandlerRT =
	std::variant<std::monostate, ReadFunctionRT<uint8_t>, ReadFunctionRT<uint16_t>,
				 ReadFunctionRT<uint32_t>, ReadFunctionRT<uint64_t>, ReadFunctionRT<int8_t>,
				 ReadFunctionRT<int16_t>, ReadFunctionRT<int32_t>, ReadFunctionRT<int64_t>,
				 ReadFunctionRT<float32_t>>;

using WriteHandlerRT =
	std::variant<std::monostate, WriteFunctionRT<uint8_t>, WriteFunctionRT<uint16_t>,
				 WriteFunctionRT<uint32_t>, WriteFunctionRT<uint64_t>, WriteFunctionRT<int8_t>,
				 WriteFunctionRT<int16_t>, WriteFunctionRT<int32_t>, WriteFunctionRT<int64_t>,
				 WriteFunctionRT<float32_t>>;

template<typename OD>
class HandlerMapRT
{
public:
	using ReadHandlerMapRT = std::map<Address, ReadHandlerRT>;
	using WriteHandlerMapRT = std::map<Address, WriteHandlerRT>;

	HandlerMapRT() {}

private:
	static constexpr auto
	makeReadHandlerMap() -> ReadHandlerMapRT
	{
		ReadHandlerMapRT builder{};
		for (const auto& [address, entry] : OD::map)
		{
			if (entry.isReadable()) { builder.emplace(address, ReadHandlerRT{}); }
		}
		return builder;
	}

	static constexpr auto
	makeWriteHandlerMap() -> WriteHandlerMapRT
	{
		WriteHandlerMapRT builder{};
		for (const auto& [address, entry] : OD::map)
		{
			if (entry.isWritable()) { builder.emplace(address, WriteHandlerRT{}); }
		}
		return builder;
	}

	ReadHandlerMapRT readHandlers = makeReadHandlerMap();
	WriteHandlerMapRT writeHandlers = makeWriteHandlerMap();

public:
	std::optional<ReadHandlerRT>
	lookupReadHandler(Address address) const
	{
		if (readHandlers.count(address) == 0) return {};

		return readHandlers.at(address);
	}

	std::optional<WriteHandlerRT>
	lookupWriteHandler(Address address) const
	{
		if (writeHandlers.count(address) == 0) return {};
		return writeHandlers.at(address);
	}

	template<Address address, typename ReturnT>
	void
	setReadHandler(std::function<ReturnT()> func)
	{
		ReadHandlerRT handler = func;
		auto entry = OD::map.lookup(address);
		if (!entry)
		{
			MODM_LOG_ERROR << "Object not found" << modm::hex << address.index << modm::ascii << ":"
						   << address.subindex << modm::endl;
			abort();
		}

		bool accessValid = entry->isReadable();
		if (!accessValid)
		{
			MODM_LOG_ERROR << "Cannot register read handler for write-only object " << modm::hex
						   << address.index << modm::ascii << ":" << address.subindex << modm::endl;
			abort();
		}

		auto handlerIndex = ReadHandlerRT(std::function<ReturnT()>()).index();
		auto odIndex = static_cast<std::size_t>(entry->dataType);
		if (odIndex != handlerIndex)
		{
			MODM_LOG_ERROR << "Invalid read handler type for entry " << modm::hex << address.index
						   << modm::ascii << ":" << address.subindex << modm::endl;
			abort();
		}

		readHandlers[address] = handler;
	}

	template<Address address, typename Param>
	void
	setWriteHandler(std::function<SdoErrorCode(Param)> func)
	{
		WriteHandlerRT handler = func;
		auto entry = OD::map.lookup(address);
		if (!entry)
		{
			MODM_LOG_ERROR << "Object not found" << modm::hex << address.index << modm::ascii << ":"
						   << address.subindex << modm::endl;
			abort();
		}
		bool accessValid = entry->isWritable();
		if (!accessValid)
		{
			MODM_LOG_ERROR << "Cannot register write handler for read-only object " << modm::hex
						   << address.index << modm::ascii << ":" << address.subindex << modm::endl;
			abort();
		}

		auto handlerIndex = WriteHandlerRT(std::function<SdoErrorCode(Param)>()).index();
		auto odIndex = static_cast<std::size_t>(entry->dataType);
		if (odIndex != handlerIndex)
		{
			MODM_LOG_ERROR << "Invalid write handler type for entry " << modm::hex << address.index
						   << modm::ascii << ":" << address.subindex << modm::endl;
			abort();
		}

		writeHandlers[address] = handler;
	}
};

inline Value
callReadHandler(ReadHandlerRT h)
{
	static_assert(Value(std::monostate{}).index() == size_t(DataType::Empty));

	switch (DataType(h.index()))
	{
		case DataType::Empty:
			return Value{};
		case DataType::UInt8:
			return Value(std::get<ReadFunctionRT<uint8_t>>(h)());
		case DataType::UInt16:
			return Value(std::get<ReadFunctionRT<uint16_t>>(h)());
		case DataType::UInt32:
			return Value(std::get<ReadFunctionRT<uint32_t>>(h)());
		case DataType::UInt64:
			return Value(std::get<ReadFunctionRT<uint64_t>>(h)());
		case DataType::Int8:
			return Value(std::get<ReadFunctionRT<int8_t>>(h)());
		case DataType::Int16:
			return Value(std::get<ReadFunctionRT<int16_t>>(h)());
		case DataType::Int32:
			return Value(std::get<ReadFunctionRT<int32_t>>(h)());
		case DataType::Int64:
			return Value(std::get<ReadFunctionRT<int64_t>>(h)());
		case DataType::Real32:
			return Value(std::get<ReadFunctionRT<float32_t>>(h)());
	}
	return Value{};
}

inline SdoErrorCode
callWriteHandler(WriteHandlerRT h, Value value)
{
	switch (DataType(h.index()))
	{
		case DataType::UInt8:
			return std::get<WriteFunctionRT<uint8_t>>(h)(*std::get_if<uint8_t>(&value));
		case DataType::UInt16:
			return std::get<WriteFunctionRT<uint16_t>>(h)(*std::get_if<uint16_t>(&value));
		case DataType::UInt32:
			return std::get<WriteFunctionRT<uint32_t>>(h)(*std::get_if<uint32_t>(&value));
		case DataType::UInt64:
			return std::get<WriteFunctionRT<uint64_t>>(h)(*std::get_if<uint64_t>(&value));
		case DataType::Int8:
			return std::get<WriteFunctionRT<int8_t>>(h)(*std::get_if<int8_t>(&value));
		case DataType::Int16:
			return std::get<WriteFunctionRT<int16_t>>(h)(*std::get_if<int16_t>(&value));
		case DataType::Int32:
			return std::get<WriteFunctionRT<int32_t>>(h)(*std::get_if<int32_t>(&value));
		case DataType::Int64:
			return std::get<WriteFunctionRT<int64_t>>(h)(*std::get_if<int64_t>(&value));
		case DataType::Real32:
			return std::get<WriteFunctionRT<float32_t>>(h)(*std::get_if<float32_t>(&value));
		case DataType::Empty:
			break;
	}
	return SdoErrorCode::GeneralError;
}

}  // namespace modm_canopen

#endif  // CANOPEN_HANDLER_MAP_RT_HPP
