#pragma once
#include <cstdint>

namespace modm_canopen
{
namespace cia402
{
enum OperatingMode : int8_t
{
	Disabled = 0,
	Voltage = -1,
	Current = -2,
	Position = 1,  // Profile position mode
	Velocity = 3
};

const char*
operatingModeToString(OperatingMode mode);
}  // namespace cia402
}  // namespace modm_canopen