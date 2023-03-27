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
		case OperatingMode::Voltage:
			return "Voltage";
		case OperatingMode::Position:
			return "Position";
		case OperatingMode::Velocity:
			return "Velocity";
	}
	return "";  // Stop my compiler whining
}

}  // namespace cia402
}  // namespace modm_canopen