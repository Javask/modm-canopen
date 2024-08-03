#pragma once
#include "handler_map_rt.hpp"
#include "overloaded.hpp"

namespace modm_canopen
{
class GenericHandlerMapRT
{

public:
	using ReadHandlerMapRT = std::map<Address, ReadHandlerRT>;
	using WriteHandlerMapRT = std::map<Address, WriteHandlerRT>;

	GenericHandlerMapRT() {}

private:
	ReadHandlerMapRT readHandlers{};
	WriteHandlerMapRT writeHandlers{};

public:
	template<typename ReturnT>
	void
	setReadHandler(Address address, std::function<std::optional<ReturnT>()> func)
	{
		readHandlers[address] = func;
	}

	template<typename Param>
	void
	setWriteHandler(Address address, std::function<SdoErrorCode(Param)> func)
	{
		writeHandlers[address] = func;
	}

	template<typename OD>
	HandlerMapRT<OD>
	bake()
	{
		HandlerMapRT<OD> map;
		for (auto &entry : readHandlers)
		{
			auto key = entry.first;
			std::visit(overloaded{[](std::monostate) { return; },
								  [&map, &key](auto &&value) {
									  map.setReadHandler(key, value);
									  return;
								  }},
					   entry.second);
		}
		for (auto &entry : writeHandlers)
		{
			auto key = entry.first;
			std::visit(overloaded{[](std::monostate) { return; },
								  [&map, &key](auto &&value) {
									  map.setWriteHandler(key, value);
									  return;
								  }},
					   entry.second);
		}
		return map;
	}
};
}  // namespace modm_canopen