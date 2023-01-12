#include "state_machine.hpp"
#include <modm/debug/logger.hpp>

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
			MODM_LOG_DEBUG << "Updated State to " << stateToString(state_) << modm::endl;
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
			if (state_ == s.state) return false;
			status_ = value;
			state_ = s.state;
			MODM_LOG_DEBUG << "Updated State to " << stateToString(state_) << modm::endl;
			return true;
		}
	}
	return false;
}

}  // namespace cia402
}  // namespace modm_canopen