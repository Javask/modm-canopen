#pragma once
#include <cstdint>
namespace modm_canopen
{
namespace cia402
{

// Cut down cia402 state space
// Ignoring states we currently do not implement:
// QuickStopActive
// FaultReactionActive
// NotReadyToSwitchOn
enum class State
{
	Invalid,
	SwitchOnDisabled,
	ReadyToSwitchOn,
	SwitchedOn,
	OperationEnabled,
	Fault
};

const char*
stateToString(State state);
}  // namespace cia402
}  // namespace modm_canopen