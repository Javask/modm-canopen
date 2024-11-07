#include "state_machine.hpp"
#include "command_bits.hpp"
#include <modm/debug/logger.hpp>
namespace modm_canopen
{
namespace cia402
{

StateMachine::StateMachine(State initial) : status_{0}, state_{initial} {}

void
StateMachine::startFaultReaction()
{
	MODM_LOG_INFO << "Changed state from " << stateToString(state_) << " to "
				  << stateToString(State::FaultReactionActive) << modm::endl;
	state_ = State::FaultReactionActive;
}

void
StateMachine::setReactionDone()
{
	State destination = state_;
	if (state_ == State::FaultReactionActive)
	{
		destination = State::Fault;
	} else
	{
		// Move from all internal state to their neighbour
		destination = (State)((uint16_t)state_ & StateMask);
	}
	if (destination != state_)
	{
		MODM_LOG_INFO << "Changed state from " << stateToString(state_) << " to "
					  << stateToString(destination) << modm::endl;
		state_ = destination;
	}
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
available(const Command& s, State state_)
{
	state_ = (State)((uint16_t)state_ & StateMask);
	return s.availableIn[0] == state_ || s.availableIn[1] == state_ || s.availableIn[2] == state_;
}

bool
StateMachine::update(uint16_t controlWord)
{
	if (control_ == controlWord) return true;
	auto oldControlWord = control_;
	control_ = controlWord;
	for (auto& s : StateCommands)
	{
		if (s.matches(controlWord, oldControlWord))
		{
			if (available(s, state_))
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
					return true;
				}
			}
		}
	}

	if ((oldControlWord & StateMask) != (controlWord & StateMask)) return false;

	if ((std::to_underlying(CommandBits::Halt) & controlWord) != 0 &&
		(std::to_underlying(CommandBits::Halt) & oldControlWord) == 0)
	{
		if (state_ == State::OperationEnabled)
		{
			MODM_LOG_INFO << "Changed state from " << stateToString(state_) << " to "
						  << stateToString(State::HaltReactionActive) << modm::endl;
			state_ = State::HaltReactionActive;
		}
	}
	return true;
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

bool
StateMachine::wasChanged()
{
	auto ret = changed_;
	changed_ = false;
	return ret;
}

}  // namespace cia402
}  // namespace modm_canopen