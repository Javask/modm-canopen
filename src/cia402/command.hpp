#pragma once
#include <cstdint>
#include "states.hpp"

namespace modm_canopen
{
namespace cia402
{

enum class CommandName : uint8_t
{
	Shutdown = 0,
	SwitchOn = 1,
	DisableVoltage = 2,
	DisableOperation = 3,
	EnableOperation = 4,
	FaultReset = 5,
	QuickStop = 6,
};

struct Command
{
	CommandName name;

	uint16_t bitMask;
	uint16_t value;
	uint16_t changing;

	State destination;
	State availableIn[3];

	inline bool
	matches(uint16_t newValue, uint16_t oldValue) const
	{  // Assume only one changing bit, will
		// trigger on any
		return ((newValue & bitMask) == value) &&
			   (changing == 0 || (oldValue & changing) != (newValue & changing));
	}

	inline uint16_t
	apply(uint16_t oldValue) const
	{
		return (oldValue & ~bitMask) | (value & bitMask);
	}
};


const char*
commandNameToString(CommandName name);

}  // namespace cia402
}  // namespace modm_canopen