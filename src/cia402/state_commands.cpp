#include "state_commands.hpp"
namespace modm_canopen::cia402
{

const char*
stateCommandNameToString(StateCommandNames name)
{
	switch (name)
	{
		case StateCommandNames::DisableOperation:
			return "DisableOperation";
		case StateCommandNames::DisableVoltage:
			return "DisableVoltage";
		case StateCommandNames::EnableOperation:
			return "EnableOperation";
		case StateCommandNames::FaultReset:
			return "FaultReset";
		case StateCommandNames::QuickStop:
			return "QuickStop";
		case StateCommandNames::Shutdown:
			return "Shutdown";
		case StateCommandNames::SwitchOn:
			return "SwitchOn";
	}
	return "Invalid";
}
}  // namespace modm_canopen::cia402