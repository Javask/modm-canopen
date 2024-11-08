#pragma once
#include <cstdint>
namespace modm_canopen::cia402
{
// Subsets supported depending on option code
enum class OptionCode : int16_t
{
	DisableDrive = 0,  // Free wheeling in Halt option code
	SlowDownWithRamp = 1,
	SlowDownWithQuickStopRamp = 2,
	// SlowDownOnCurrentLimit = 3,
	// SlowDownOnVoltageLimit = 4,
	SlowDownWithRampAndStay = 5,
	SlowDownWithQuickStopRampAndStay = 6,
	// SlowDownOnCurrentLimitAndStay = 7,
	// SlowDownOnVoltageLimitAndStay = 8,
};

bool
isValidOptionCode(const OptionCode& code);
}  // namespace modm_canopen::cia402