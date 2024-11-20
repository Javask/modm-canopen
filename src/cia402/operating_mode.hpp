#pragma once
#include <cstdint>

namespace modm_canopen
{
namespace cia402
{
enum OperatingMode : int8_t
{
	Disabled = 0,
	ProfilePosition = 1,
	Velocity = 2,
	ProfileVelocity = 3,
	ProfileTorque = 4,
	Homing = 6,
	InterpolatedPosition = 7,
	CyclicSynchronousPosition = 8,
	CyclicSynchronousVelocity = 9,
	CyclicSynchronousTorque = 10,
};

const char*
operatingModeToString(OperatingMode mode);
}  // namespace cia402
}  // namespace modm_canopen