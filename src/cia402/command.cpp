#include "command.hpp"
namespace modm_canopen::cia402
{

const char*
commandNameToString(CommandName name)
{
	switch (name)
	{
		case CommandName::DisableOperation:
			return "DisableOperation";
		case CommandName::DisableVoltage:
			return "DisableVoltage";
		case CommandName::EnableOperation:
			return "EnableOperation";
		case CommandName::FaultReset:
			return "FaultReset";
		case CommandName::QuickStop:
			return "QuickStop";
		case CommandName::Shutdown:
			return "Shutdown";
		case CommandName::SwitchOn:
			return "SwitchOn";
	}
	return "Invalid";
}
}  // namespace modm_canopen::cia402