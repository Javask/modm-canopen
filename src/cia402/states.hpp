#pragma once
#include <cstdint>
namespace modm_canopen
{
namespace cia402
{

// Cut down cia402 state space
// Ignoring states we currently do not implement:
// NotReadyToSwitchOn
constexpr uint16_t StateMask = 0x006F;
enum class State : uint16_t
{
	Invalid = 0x0000,
	SwitchOnDisabled = 0x0040,
	ReadyToSwitchOn = 0x0021,
	SwitchedOn = 0x0023,
	OperationEnabled = 0x0027,
	QuickStopActive = 0x0007,
	FaultReactionActive = 0x000F,
	Fault = 0x0008,
};

const char*
stateToString(State state);
}  // namespace cia402
}  // namespace modm_canopen