#ifndef CANOPEN_CIA402_PROTOCOL_HPP
#define CANOPEN_CIA402_PROTOCOL_HPP
#include <modm/architecture/interface/can_message.hpp>
#include "../device/handler_map.hpp"
#include "cia402_objects.hpp"
#include "operating_mode.hpp"
#include "command_word.hpp"
#include "state_machine.hpp"
#include "factors.hpp"
#include "../sdo_error.hpp"
#include <cstdint>

namespace modm_canopen::cia402
{
class CiA402
{
private:
	static inline OperatingMode mode_{OperatingMode::Disabled};
	static inline StateMachine status_{State::SwitchOnDisabled};
	static inline CommandWord control_{0};

	static constexpr uint32_t supportedModesBitfield_ = 0b1110'0000'0000'0000'0000'0000'0110'1111;
	static inline bool
	isSupported(OperatingMode mode);

public:
	template<typename Device, typename MessageCallback>
	static void
	update(MessageCallback&&);

	template<typename Device, typename MessageCallback>
	static void
	processMessage(const modm::can::Message&, MessageCallback&&);

	template<typename ObjectDictionary>
	constexpr void
	registerHandlers(HandlerMap<ObjectDictionary>& map);
};
}  // namespace modm_canopen::cia402

#include "cia402_protocol_impl.hpp"
#endif