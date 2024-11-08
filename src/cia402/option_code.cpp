#include "option_code.hpp"

namespace modm_canopen::cia402
{
bool
isValidOptionCode(const OptionCode& code)
{
	switch (code)
	{
		case OptionCode::DisableDrive:
		case OptionCode::SlowDownWithRamp:
		case OptionCode::SlowDownWithQuickStopRamp:
		case OptionCode::SlowDownWithRampAndStay:
		case OptionCode::SlowDownWithQuickStopRampAndStay:
			return true;
		default:
			return false;
	}
}
}  // namespace modm_canopen::cia402