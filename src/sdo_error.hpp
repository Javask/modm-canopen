#ifndef CANOPEN_SDO_ERROR_HPP
#define CANOPEN_SDO_ERROR_HPP

#include <cstdint>

namespace modm_canopen
{

enum class SdoErrorCode : uint32_t
{
	NoError = 0,
	ToggleBitNotAlternated = 0x0503'0000,
	SDOProtocolTimedOut = 0x0504'0000,
	InvalidCommand = 0x0504'0001,
	InvalidBlockSize = 0x0504'0002,       // Block mode only
	InvalidSequenceNumber = 0x0504'0003,  // Block mode only
	CRCError = 0x0504'0004,               // Block mode only
	OutOfMemory = 0x0504'0005,
	UnsupportedAccess = 0x0601'0000,
	ReadOfWriteOnlyObject = 0x0601'0001,
	WriteOfReadOnlyObject = 0x0601'0002,
	ObjectDoesNotExist = 0x0602'0000,
	PdoMappingError = 0x0604'0041,
	MappingsExceedPdoLength = 0x0604'0042,
	GeneralParameterIncompatibility = 0x0604'0043,
	GeneralInternalIncompatibility = 0x0604'0047,
	HardwareError = 0x0606'0000,
	DataTypeDoesNotMatchLengthDoesNotMatch = 0x0607'0010,
	DataTypeDoesNotMatchLengthTooHigh = 0x0607'0012,
	DataTypeDoesNotMatchLengthTooLow = 0x0607'0013,
	SubIndexDoesNotExist = 0x0609'0011,
	InvalidValue = 0x0609'0030,             // Download only
	ValueOfParameterTooHigh = 0x0609'0031,  // Download only
	ValueOfParameterTooLow = 0x0609'0032,   // Download only
	MaximumValueLessThanMinimumValue = 0x0609'0036,
	ResourceUnavailable = 0x060A'0023,
	GeneralError = 0x0800'0000,
	DataCannotBeTransferred = 0x0800'0020,
	NoDataAvailable = 0x0800'0024,
};

const char*
sdoErrorToString(SdoErrorCode code);

}  // namespace modm_canopen

#endif
