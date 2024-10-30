#include "emcy_error.hpp"
namespace modm_canopen
{

const char*
emcyErrorToString(EMCYError code)
{
	switch (code)
	{
		case EMCYError::NoError:
			return "No Error";
		case EMCYError::GenericError:
			return "Generic Error";
		case EMCYError::GenericCurrentError:
			return "Generic Current Error";
		case EMCYError::GenericCurrentInputError:
			return "Generic Current Input Error";
		case EMCYError::GenericCurrentInternalError:
			return "Generic Current Internal Error";
		case EMCYError::GenericCurrentOutputError:
			return "Generic Current Output Error";
		case EMCYError::GenericVoltageError:
			return "Generic Voltage Error";
		case EMCYError::GenericVoltageInputError:
			return "Generic Voltage Input Error";
		case EMCYError::GenericVoltageInternalError:
			return "Generic Voltage Internal Error";
		case EMCYError::GenericVoltageOutputError:
			return "Generic Voltage Output Error";
		case EMCYError::GenericTemperatureError:
			return "Generic Temperature Error";
		case EMCYError::GenericAmbientTemperatureError:
			return "Generic Ambient Temperature Error";
		case EMCYError::GenericDeviceTemperatureError:
			return "Generic Device Temperature Error";
		case EMCYError::GenericHardwareError:
			return "Generic Hardware Error";
		case EMCYError::GenericSoftwareError:
			return "Generic Software Error";
		case EMCYError::GenericInternalSoftwareError:
			return "Generic Internal Software Error";
		case EMCYError::GenericUserSoftwareError:
			return "Generic User Software Error";
		case EMCYError::GenericDataSetError:
			return "Generic Dataset Error";
		case EMCYError::GenericMonitoringError:
			return "Generic Monitoring Error";
		case EMCYError::GenericCommunicationError:
			return "Generic Communication Error";
		case EMCYError::CanOverrun:
			return "Can Overrun";
		case EMCYError::CanInErrorPassiveMode:
			return "Can in Error Passive Mode";
		case EMCYError::HeartbeartOrGuardingError:
			return "Heartbeart or Guarding Error";
		case EMCYError::RecoveredFromBusOff:
			return "Recovered from Bus off";
		case EMCYError::CanIDCollision:
			return "Can ID Collision";
		case EMCYError::GenericProtocolError:
			return "Generic Protocol Error";
		case EMCYError::PDODroppedDueToLengthError:
			return "PDO Dropped Due To Length Error";
		case EMCYError::PDOLenghtExceeded:
			return "PDO Lenght Exceeded";
		case EMCYError::DAMPDONotProcessed:
			return "DAM PDO Not Processed";
		case EMCYError::UnexpectedSYNCData:
			return "Unexpected SYNC Data";
		case EMCYError::RPDOTimeout:
			return "RPDO Timeout";
		case EMCYError::GenericExternalError:
			return "Generic External Error";
		case EMCYError::GenericAdditionalFunctionError:
			return "Generic Additional Function Error";
		case EMCYError::GenericDeviceSpecificError:
			return "Generic Device Specific Error";
		default:
			return "Unknown Error";
	}
}
}  // namespace modm_canopen