#include "state_machine.hpp"
namespace modm_canopen
{
namespace cia402
{
StateMachine::StateMachine(State initial) : status_{0}, state_{initial}
{
	for (auto& s : StateInfos)
	{
		if (s.state == initial) { status_ = s.apply(status_); }
	}
}

StateMachine::StateMachine(uint16_t raw)
{
	state_ = parseState(raw);
	status_ = raw;
}

State
StateMachine::parseState(uint16_t val)
{
	for (auto& s : StateInfos)
	{
		if (s.matches(val)) { return s.state; }
	}
	return State::Invalid;
}

bool
StateMachine::update(const CommandWord& cmd)
{
	auto destination_ = State::Invalid;
	for (auto& s : StateCommands)
	{
		if (cmd.matches(s.cmd))
		{
			if (s.availableIn[0] == state_ || s.availableIn[1] == state_ ||
				s.availableIn[2] == state_)
			{
				destination_ = s.destination;
				break;
			}
		}
	}
	if (destination_ == State::Invalid) return false;

	for (auto& s : StateInfos)
	{
		if (s.state == destination_)
		{
			status_ = s.apply(status_);
			state_ = s.state;
			return true;
		}
	}

	return false;
}

bool
StateMachine::set(uint16_t value)
{
	for (auto& s : StateInfos)
	{
		if (s.matches(value))
		{
			status_ = value;
			if (state_ == s.state) return true;
			state_ = s.state;
			return true;
		}
	}
	return false;
}

}  // namespace cia402
}  // namespace modm_canopen