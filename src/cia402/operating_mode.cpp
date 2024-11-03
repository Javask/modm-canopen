#include "operating_mode.hpp"

namespace modm_canopen
{
namespace cia402
{

const char*
operatingModeToString(OperatingMode mode)
{

	switch (mode)
	{
		case OperatingMode::Disabled:
			return "Disabled";
		case OperatingMode::ProfilePosition:
			return "ProfilePosition";
		case OperatingMode::Velocity:
			return "Velocity";
		case OperatingMode::ProfileVelocity:
			return "ProfileVelocity";
		case OperatingMode::ProfileTorque:
			return "ProfileTorque";
		case OperatingMode::Homing:
			return "Homing";
		case OperatingMode::InterpolatedPosition:
			return "InterpolatedPosition";
		default:
			return "Unknown";
	}
}

}  // namespace cia402
}  // namespace modm_canopen