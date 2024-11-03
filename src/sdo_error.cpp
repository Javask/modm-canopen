#include "sdo_error.hpp"

namespace modm_canopen
{

const char*
sdoErrorToString(SdoErrorCode code)
{
	switch (code)
	{
		case SdoErrorCode::NoError:
			return "No error";
		case SdoErrorCode::ToggleBitNotAlternated:
			return "Toggle bit not alternated";
		case SdoErrorCode::SDOProtocolTimedOut:
			return "SDO Protocol timed out";
		case SdoErrorCode::InvalidCommand:
			return "Invalid command";
		case SdoErrorCode::InvalidBlockSize:
			return "Invalid block size";
		case SdoErrorCode::InvalidSequenceNumber:
			return "Invalid sequence number";
		case SdoErrorCode::CRCError:
			return "CRC Error";
		case SdoErrorCode::OutOfMemory:
			return "Out of memory";
		case SdoErrorCode::UnsupportedAccess:
			return "Unsupported access";
		case SdoErrorCode::ReadOfWriteOnlyObject:
			return "Read of write only object";
		case SdoErrorCode::WriteOfReadOnlyObject:
			return "Write of read only object";
		case SdoErrorCode::ObjectDoesNotExist:
			return "Object does not exist";
		case SdoErrorCode::PdoMappingError:
			return "PDO mapping error";
		case SdoErrorCode::MappingsExceedPdoLength:
			return "Mapping exceeds pdo length";
		case SdoErrorCode::GeneralParameterIncompatibility:
			return "General parameter incompatibility";
		case SdoErrorCode::GeneralInternalIncompatibility:
			return "General internal incompatibility";
		case SdoErrorCode::HardwareError:
			return "Hardware error";
		case SdoErrorCode::DataTypeDoesNotMatchLengthDoesNotMatch:
			return "DataType does not match, length does not match";
		case SdoErrorCode::DataTypeDoesNotMatchLengthTooHigh:
			return "DataType does not match, length too high";
		case SdoErrorCode::DataTypeDoesNotMatchLengthTooLow:
			return "DataType does not match, length too low";
		case SdoErrorCode::SubIndexDoesNotExist:
			return "SubIndex does not exist";
		case SdoErrorCode::InvalidValue:
			return "Invalid value";
		case SdoErrorCode::ValueOfParameterTooHigh:
			return "Value of parameter too high";
		case SdoErrorCode::ValueOfParameterTooLow:
			return "Value of parameter too low";
		case SdoErrorCode::MaximumValueLessThanMinimumValue:
			return "Maximum value less than minimum value";
		case SdoErrorCode::ResourceUnavailable:
			return "Resource unavailable";
		case SdoErrorCode::GeneralError:
			return "General error";
		case SdoErrorCode::DataCannotBeTransferred:
			return "Data cannot be transferred";
		case SdoErrorCode::NoDataAvailable:
			return "No data available";
		default:
			return "Not in switch-case";
	}
}
}  // namespace modm_canopen