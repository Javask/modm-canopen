#include "states.hpp"

namespace modm_canopen
{

namespace cia402
{

const char*
stateToString(State state)
{
	switch (state)
	{
		case State::Fault:
			return "Fault";
		case State::Invalid:
			return "Invalid";
		case State::OperationEnabled:
			return "OperationEnabled";
		case State::ReadyToSwitchOn:
			return "ReadyToSwitchOn";
		case State::SwitchedOn:
			return "SwitchedOn";
		case State::SwitchOnDisabled:
			return "SwitchOnDisabled";
		case State::QuickStopActive:
			return "QuickStopActive";
		case State::FaultReactionActive:
			return "FaultReactionActive";
		case State::HaltReactionActive:
			return "HaltReactionActive";
		case State::DisableReactionActive:
			return "DisableReactionActive";
		case State::ShutdownReactionActive:
			return "ShutdownReactionActive";
	}
	return "";  // Stop my compiler whining
}
}  // namespace cia402
}  // namespace modm_canopen