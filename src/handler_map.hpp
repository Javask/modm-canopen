#ifndef CANOPEN_HANDLER_MAP_HPP
#define CANOPEN_HANDLER_MAP_HPP

#include <cstdint>
#include <variant>
#include "float_types.hpp"
#include "sdo_error.hpp"
#include "constexpr_map.hpp"
#include "handlers.hpp"

namespace modm_canopen
{

template<typename OD>
class HandlerMap
{
public:
	static constexpr std::size_t ReadHandlerCount = readableEntryCount<OD>();
	static constexpr std::size_t WriteHandlerCount = writableEntryCount<OD>();

	using ReadHandlerMap = ConstexprMap<Address, ReadHandler, ReadHandlerCount>;
	using WriteHandlerMap = ConstexprMap<Address, WriteHandler, WriteHandlerCount>;

	constexpr HandlerMap() {}

private:
	static constexpr auto
	makeReadHandlerMap() -> ReadHandlerMap
	{
		ConstexprMapBuilder<Address, ReadHandler, ReadHandlerCount> builder{};
		for (const auto& [address, entry] : OD::map)
		{
			if (entry.isReadable()) { builder.insert(address, ReadHandler{}); }
		}
		return builder.buildMap();
	}

	static constexpr auto
	makeWriteHandlerMap() -> WriteHandlerMap
	{
		ConstexprMapBuilder<Address, WriteHandler, WriteHandlerCount> builder{};
		for (const auto& [address, entry] : OD::map)
		{
			if (entry.isWritable()) { builder.insert(address, WriteHandler{}); }
		}
		return builder.buildMap();
	}

	ReadHandlerMap readHandlers = makeReadHandlerMap();
	WriteHandlerMap writeHandlers = makeWriteHandlerMap();

public:
	constexpr auto
	lookupReadHandler(Address address) const
	{
		return readHandlers.lookup(address);
	}

	constexpr auto
	lookupWriteHandler(Address address) const
	{
		return writeHandlers.lookup(address);
	}

	template<Address address, typename ReturnT>
	constexpr void
	setReadHandler(ReturnT (*func)())
	{
		ReadHandler handler = func;
		constexpr auto entry = OD::map.lookup(address);
		static_assert(entry, "Object not found");

		// if constexpr prevents ugly compiler output when assertion triggers
		if constexpr (entry)
		{
			constexpr bool accessValid = entry->isReadable();
			static_assert(accessValid, "Cannot register read handler for write-only object");

			constexpr auto handlerIndex = ReadHandler(static_cast<ReturnT (*)()>(nullptr)).index();
			constexpr auto odIndex = static_cast<std::size_t>(entry->dataType);
			static_assert(odIndex == handlerIndex, "Invalid read handler type for entry");

			if constexpr (accessValid && (odIndex == handlerIndex))
			{
				*readHandlers.lookup(address) = handler;
			}
		}
	}

	template<Address address, typename Param>
	constexpr void
	setWriteHandler(SdoErrorCode (*func)(Param))
	{
		WriteHandler handler = func;
		constexpr auto entry = OD::map.lookup(address);
		static_assert(entry, "Object not found");

		// if constexpr prevents ugly compiler output when assertion triggers
		if constexpr (entry)
		{
			constexpr bool accessValid = entry->isWritable();
			static_assert(accessValid, "Cannot register write handler for read-only object");

			constexpr auto odIndex = static_cast<std::size_t>(entry->dataType);
			constexpr auto handlerIndex =
				WriteHandler(static_cast<SdoErrorCode (*)(Param)>(nullptr)).index();
			static_assert(odIndex == handlerIndex, "Invalid write handler type for entry");

			if constexpr (accessValid && (odIndex == handlerIndex))
			{
				*writeHandlers.lookup(address) = handler;
			}
		}
	}
};

template<typename OD>
constexpr Address
findMissingReadHandler(const HandlerMap<OD>& map)
{
	for (const auto& entry : OD::map)
	{
		if (entry.second.isReadable())
		{
			auto lookup = map.lookupReadHandler(entry.second.address);
			if (!lookup || std::holds_alternative<std::monostate>(*lookup))
			{
				return entry.second.address;
			}
		}
	}
	return Address{};
}

template<typename OD>
constexpr Address
findMissingWriteHandler(const HandlerMap<OD>& map)
{
	for (const auto& entry : OD::map)
	{
		if (entry.second.isWritable())
		{
			auto lookup = map.lookupWriteHandler(entry.second.address);
			if (!lookup || std::holds_alternative<std::monostate>(*lookup))
			{
				return entry.second.address;
			}
		}
	}
	return Address{};
}
}  // namespace modm_canopen

#endif  // CANOPEN_HANDLER_MAP_HPP
