#pragma once
#include "command.hpp"
#include "command_bits.hpp"

namespace modm_canopen
{
namespace cia402
{
class CommandWord
{
private:
	uint16_t value_{};
	uint16_t oldValue_{};

public:
	explicit inline CommandWord(uint16_t initial) : value_{initial}, oldValue_{initial} {};

	inline uint16_t
	value() const
	{
		return value_;
	};

	inline bool
	matches(const Command& cmd) const
	{
		return cmd.matches(value_, oldValue_);
	}

	inline void
	update(const uint16_t newValue)
	{
		oldValue_ = value_;
		value_ = newValue;
	}

	inline void
	apply(const Command& cmd)
	{
		update(cmd.apply(value_));
	}

	template<CommandBits bit>
	inline bool
	isSet()
	{
		return (value_ & ((uint16_t)bit)) == ((uint16_t)bit);
	}

	template<CommandBits bit>
	inline void
	setBit(bool value)
	{
		value_ = (value_ & ~(uint16_t)bit) | (value ? (uint16_t)bit : 0);
	}


};
}  // namespace cia402
}  // namespace modm_canopen