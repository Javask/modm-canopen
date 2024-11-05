#pragma once
#include <cstdint>
#include <utility>
#include "states.hpp"
#include "state_commands.hpp"
#include "status_bits.hpp"

namespace modm_canopen
{
namespace cia402
{

class StateMachine
{
private:
	uint16_t status_;
	uint16_t control_;
	State state_;
	CommandName lastCommand_;

public:
	static State
	parseState(uint16_t val);

	void
	startFaultReaction();
	void
	setFaultReactionStopped();

	explicit StateMachine(State initial);
	explicit StateMachine(uint16_t raw);

	bool
	update(uint16_t controlWord);

	bool
	set(uint16_t statusWord);

	inline uint16_t
	status() const
	{
		return (status_ & ~StateMask) | std::to_underlying(state_);
	}

	inline uint16_t
	control() const
	{
		return control_;
	}

	inline CommandName
	lastCommand() const
	{
		return lastCommand_;
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
		return (status() & std::to_underlying(bit)) == std::to_underlying(bit);
	}

	template<StatusBits bit>
	inline void
	setBit(bool value)
	{
		// Dont accidentally change state
		static_assert(std::to_underlying(bit) & StateMask == 0);
		status_ = (status_ & ~std::to_underlying(bit)) | (value ? std::to_underlying(bit) : 0);
	}
};

}  // namespace cia402
}  // namespace modm_canopen