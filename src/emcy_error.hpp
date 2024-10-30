#pragma once
#include <cstdint>
namespace modm_canopen
{
enum class EMCYError : uint16_t
{
	NoError = 0x0000,
	GenericError = 0x1000,
	GenericCurrentError = 0x2000,
	GenericCurrentInputError = 0x2100,
	GenericCurrentInternalError = 0x2200,
	GenericCurrentOutputError = 0x2300,
	GenericVoltageError = 0x3000,
	GenericVoltageInputError = 0x3100,
	GenericVoltageInternalError = 0x3200,
	GenericVoltageOutputError = 0x3300,
	GenericTemperatureError = 0x4000,
	GenericAmbientTemperatureError = 0x4100,
	GenericDeviceTemperatureError = 0x4200,
	GenericHardwareError = 0x5000,
	GenericSoftwareError = 0x6000,
	GenericInternalSoftwareError = 0x6100,
	GenericUserSoftwareError = 0x6200,
	GenericDataSetError = 0x6300,
	GenericMonitoringError = 0x8000,
	GenericCommunicationError = 0x8100,
	CanOverrun = 0x8110,
	CanInErrorPassiveMode = 0x8120,
	HeartbeartOrGuardingError = 0x8130,
	RecoveredFromBusOff = 0x8140,
	CanIDCollision = 0x8150,
	GenericProtocolError = 0x8200,
	PDODroppedDueToLengthError = 0x8210,
	PDOLenghtExceeded = 0x8220,
	DAMPDONotProcessed = 0x8230,
	UnexpectedSYNCData = 0x8240,
	RPDOTimeout = 0x8250,
	GenericExternalError = 0x9000,
	GenericAdditionalFunctionError = 0xF000,
	GenericDeviceSpecificError = 0xFF00,
};

const char*
emcyErrorToString(EMCYError code);
}  // namespace modm_canopen