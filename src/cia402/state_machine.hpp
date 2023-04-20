#pragma once
#include <cstdint>
#include "states.hpp"
#include "command_word.hpp"
#include "state_commands.hpp"
#include "status_bits.hpp"

namespace modm_canopen
{
namespace cia402
{

class StateMachine
{
private:
	struct StateInfo
	{
		State state;
		uint16_t mask;
		uint16_t value;

		inline bool
		matches(uint16_t status) const
		{
			return (status & mask) == value;
		}

		inline uint16_t
		apply(uint16_t status) const
		{
			return (status & ~mask) | (value & mask);
		}
	};

	static inline constexpr std::array<StateInfo, 6> StateInfos{
		StateInfo{.state{State::Fault}, .mask{0b0100'1111}, .value{0b0000'1000}},
		StateInfo{.state{State::OperationEnabled}, .mask{0b0110'1111}, .value{0b0010'0111}},
		StateInfo{.state{State::ReadyToSwitchOn}, .mask{0b0110'1111}, .value{0b0010'0001}},
		StateInfo{.state{State::SwitchedOn}, .mask{0b0110'1111}, .value{0b0010'0011}},
		StateInfo{.state{State::SwitchOnDisabled}, .mask{0b0100'1111}, .value{0b0100'0000}},
		StateInfo{.state{State::QuickStopActive}, .mask{0b0110'1111}, .value{0b0000'0111}},
	};

	uint16_t status_;
	State state_;

public:
	explicit StateMachine(State initial);

	bool
	update(const CommandWord& cmdWord);

	bool
	set(uint16_t value);

	inline uint16_t
	status() const
	{
		return status_;
	}

	inline State
	state() const
	{
		return state_;
	}

	template<StatusBits bit>
	inline bool
	isSet() const
	{
		return (status_ & ((uint16_t)bit)) == ((uint16_t)bit);
	}

	template<StatusBits bit>
	inline void
	setBit(bool value)
	{
		status_ = (status_ & ~(uint16_t)bit) | (value ? (uint16_t)bit : 0);
	}
};

}  // namespace cia402
}  // namespace modm_canopen