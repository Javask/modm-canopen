#include "sdo_error.hpp"

namespace modm_canopen
{

const char*
sdoErrorToString(SdoErrorCode code)
{
	switch (code)
	{
		case SdoErrorCode::NoError:
			return "No Error";
		case SdoErrorCode::UnsupportedAccess:
			return "Unsupported Access";
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
		case SdoErrorCode::InvalidValue:
			return "Invalid value";
		case SdoErrorCode::GeneralError:
			return "General error";
		default:
			return "Not in switch-case";
	}
}
}  // namespace modm_canopen