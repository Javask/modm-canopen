#pragma once
#include <cstdint>

namespace modm_canopen
{
namespace cia402
{

struct Command
{
	uint16_t bitMask;
	uint16_t value;
	uint16_t changing;

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

}  // namespace cia402
}  // namespace modm_canopen