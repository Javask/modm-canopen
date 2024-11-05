#include "state_machine.hpp"
#include <modm/debug/logger.hpp>
namespace modm_canopen
{
namespace cia402
{
StateMachine::StateMachine(State initial) : status_{0}, state_{initial} {}

void
StateMachine::startFaultReaction()
{
	state_ = State::FaultReactionActive;
}
void
StateMachine::setFaultReactionStopped()
{
	if (state_ == State::FaultReactionActive) { state_ = State::Fault; }
}

StateMachine::StateMachine(uint16_t raw)
{
	state_ = parseState(raw);
	status_ = raw & ~StateMask;
}

State
StateMachine::parseState(uint16_t val)
{
	switch (val & StateMask)
	{
		case std::to_underlying(State::SwitchOnDisabled):
			return State::SwitchOnDisabled;
		case std::to_underlying(State::ReadyToSwitchOn):
			return State::ReadyToSwitchOn;
		case std::to_underlying(State::SwitchedOn):
			return State::SwitchedOn;
		case std::to_underlying(State::OperationEnabled):
			return State::OperationEnabled;
		case std::to_underlying(State::QuickStopActive):
			return State::QuickStopActive;
		case std::to_underlying(State::FaultReactionActive):
			return State::FaultReactionActive;
		case std::to_underlying(State::Fault):
			return State::Fault;
		case std::to_underlying(State::Invalid):
		default:
			break;
	}

	return State::Invalid;
}

bool
StateMachine::update(uint16_t controlWord)
{

	for (auto& s : StateCommands)
	{
		if (s.matches(controlWord, control_))
		{
			if (s.availableIn[0] == state_ || s.availableIn[1] == state_ ||
				s.availableIn[2] == state_)
			{
				if (s.destination != State::Invalid)
				{
					MODM_LOG_INFO << "Received command " << commandNameToString(s.name)
								  << modm::endl;
					lastCommand_ = s.name;
					if (state_ != s.destination)
					{
						MODM_LOG_INFO << "Changed state from " << stateToString(state_) << " to "
									  << stateToString(s.destination) << modm::endl;
					}
					state_ = s.destination;
					control_ = controlWord;
					return true;
				}
			}
		}
	}
	return false;
}

bool
StateMachine::set(uint16_t value)
{
	State s = parseState(value);
	if (s == State::Invalid) return false;
	status_ = value & ~StateMask;
	state_ = s;
	return true;
}

}  // namespace cia402
}  // namespace modm_canopen