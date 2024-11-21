#ifndef CANOPEN_STATE_MACHINE_HPP
#define CANOPEN_STATE_MACHINE_HPP
#include <cstdint>
#include <utility>
#include <optional>
#include "states.hpp"
#include "state_commands.hpp"
#include "status_bits.hpp"
#include "command_bits.hpp"

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
	bool changed_;


  static bool available(const Command &s, State state_);

public:
	static State
	parseState(uint16_t val);

	void
	startFaultReaction();

	void
	setReactionDone();

	explicit StateMachine(State initial);
	explicit StateMachine(uint16_t status, uint16_t control = 0);

	bool
	update(uint16_t controlWord);

	bool
	set(uint16_t statusWord);

	bool
	wasChanged();

	inline uint16_t
	status() const
	{
		return (status_ & ~StateMask) | (std::to_underlying(state_) & StateMask);
	}

	inline std::optional<uint16_t>
	getNextControlWord(CommandName cmd)
	{
		for (const auto& c : StateCommands)
		{
			if (c.name == cmd && available(c, state())) { return c.apply(control_); }
		}
		return {};
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

	template<CommandBits bit>
	inline bool
	isSetControl() const
	{
		return (control() & std::to_underlying(bit)) == std::to_underlying(bit);
	}

	template<CommandBits bit>
	inline void
	setControlBit(bool value)
	{
		// Dont accidentally change state
		control_ = (control_ & ~std::to_underlying(bit)) | (value ? std::to_underlying(bit) : 0);
	}
};

}  // namespace cia402
}  // namespace modm_canopen

#endif