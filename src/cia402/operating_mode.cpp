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
		case OperatingMode::Current:
			return "Current";
		case OperatingMode::Position:
			return "Position";
		case OperatingMode::Velocity:
			return "Velocity";
		case OperatingMode::Test:
			return "Test";
	}
	return "";  // Stop my compiler whining
}

bool
operatingModeIsValid(int8_t value)
{
	return (value == int8_t(OperatingMode::Disabled)) ||
		   (value == int8_t(OperatingMode::Voltage)) ||
		   (value == int8_t(OperatingMode::Velocity)) ||
		   (value == int8_t(OperatingMode::Position)) ||
		   (value == int8_t(OperatingMode::Current)) || (value == int8_t(OperatingMode::Test));
}

}  // namespace cia402
}  // namespace modm_canopen