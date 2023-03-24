#ifndef CANOPEN_HANDLER_MAP_RT_HPP
#define CANOPEN_HANDLER_MAP_RT_HPP

#include <cstdint>
#include <variant>
#include <cassert>
#include <map>
#include "float_types.hpp"
#include "sdo_error.hpp"
#include "handlers.hpp"

#include <modm/debug/logger.hpp>

namespace modm_canopen
{

template<typename OD>
class HandlerMapRT
{
public:
	using ReadHandlerMap = std::map<Address, ReadHandler>;
	using WriteHandlerMap = std::map<Address, WriteHandler>;

	HandlerMapRT() {}

private:
	static constexpr auto
	makeReadHandlerMap() -> ReadHandlerMap
	{
		ReadHandlerMap builder{};
		for (const auto& [address, entry] : OD::map)
		{
			if (entry.isReadable()) { builder.emplace(address, ReadHandler{}); }
		}
		return builder;
	}

	static constexpr auto
	makeWriteHandlerMap() -> WriteHandlerMap
	{
		WriteHandlerMap builder{};
		for (const auto& [address, entry] : OD::map)
		{
			if (entry.isWritable()) { builder.emplace(address, WriteHandler{}); }
		}
		return builder;
	}

	ReadHandlerMap readHandlers = makeReadHandlerMap();
	WriteHandlerMap writeHandlers = makeWriteHandlerMap();

public:
	std::optional<ReadHandler>
	lookupReadHandler(Address address) const
	{
		if (readHandlers.count(address) == 0) return {};

		return readHandlers.at(address);
	}

	std::optional<WriteHandler>
	lookupWriteHandler(Address address) const
	{
		if (writeHandlers.count(address) == 0) return {};
		return writeHandlers.at(address);
	}

	template<Address address, typename ReturnT>
	void
	setReadHandler(ReturnT (*func)())
	{
		ReadHandler handler = func;
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

		auto handlerIndex = ReadHandler(static_cast<ReturnT (*)()>(nullptr)).index();
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
	setWriteHandler(SdoErrorCode (*func)(Param))
	{
		WriteHandler handler = func;
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

		auto handlerIndex = WriteHandler(static_cast<SdoErrorCode (*)(Param)>(nullptr)).index();
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
}  // namespace modm_canopen

#endif  // CANOPEN_HANDLER_MAP_RT_HPP
