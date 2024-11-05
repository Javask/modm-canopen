#ifndef CANOPEN_CIA402_PROTOCOL_HPP
#define CANOPEN_CIA402_PROTOCOL_HPP
#include <modm/architecture/interface/can_message.hpp>
#include "../device/handler_map.hpp"
#include "cia402_objects.hpp"
#include "operating_mode.hpp"
#include "state_machine.hpp"
#include "option_code.hpp"
#include "factors.hpp"
#include "../sdo_error.hpp"
#include <cstdint>

namespace modm_canopen::cia402
{
template<uint8_t Axis>
class CiA402
{
	static_assert(Axis < 8, "Only 8 axes per device are supported by the CiA402 standard");

public:
	enum class MotorState
	{
		On,
		Idle,
		Braking
	};

	enum class ControlMode
	{
		Velocity,
		Voltage,
		Current,
		Position,
		None,
	};

	struct PidValues
	{
		float p{1.0f};
		float i{0.0f};
		float d{0.0f};
		float iSum{1.0f};
		float max{0.0f};
	};

	struct Inputs
	{
		int32_t current[3];  // ma
		int32_t torque;      // ticks/s^2? TBD
		int32_t velocity;    // ticks/s
		int32_t position;    // ticks
	};

	struct Outputs
	{
		MotorState state;
		ControlMode mode;
		union {
			int32_t torque;   // ticks/s^2
			int32_t velocity; // ticks/s
			int32_t position; // ticks
		} demand;
		uint16_t currentLimit; // ma
	};

	struct ControlValues
	{
		PidValues c;
		PidValues v;
		PidValues p;
	};

private:
	static inline OperatingMode demandedMode_{OperatingMode::Disabled};
	static inline OperatingMode displayedMode_{OperatingMode::Disabled};
	static inline StateMachine status_{State::SwitchOnDisabled};
	static inline State lastProcessedState_{State::SwitchOnDisabled};

	static inline OptionCode shutdownCode_{OptionCode::DisableDrive};
	static inline OptionCode disableCode_{OptionCode::SlowDownWithRamp};
	static inline OptionCode quickStopCode_{OptionCode::SlowDownWithQuickStopRamp};
	static inline OptionCode haltCode_{OptionCode::SlowDownWithRamp};
	static inline OptionCode faultCode_{OptionCode::SlowDownWithQuickStopRamp};

	static constexpr uint32_t supportedModesBitfield_ = 0b1110'0000'0000'0000'0000'0000'0110'1111;
	static inline bool
	isSupported(OperatingMode mode);

	static inline Outputs outputs_{};

public:
	// Input

	static void
	setError();  // TODO make an interface

	static void
	updateInputs(const Inputs& in);

	// Output

	static const Outputs&
	getOutputs();

	// Control values
	static const ControlValues&
	getControlValues();

	// Canopen Protocol
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