#ifndef CANOPEN_CIA402_PROTOCOL_HPP
#define CANOPEN_CIA402_PROTOCOL_HPP
#include <modm/architecture/interface/can_message.hpp>
#include "../device/handler_map.hpp"

namespace modm_canopen
{
class CiA402
{
public:
	template<typename Device, typename MessageCallback>
	static void
	update(MessageCallback&&);

	template<typename Device, typename MessageCallback>
	static void
	processMessage(const modm::can::Message&, MessageCallback&&);

	template<typename ObjectDictionary>
	constexpr void
	registerHandlers(modm_canopen::HandlerMap<ObjectDictionary>&);
};
}  // namespace modm_canopen

#include "cia402_protocol_impl.hpp"
#endif