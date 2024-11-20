#pragma once
#include <cstdint>
namespace modm_canopen
{
struct Identity
{
	uint32_t deviceType_;
	uint32_t vendorId_;
	uint32_t productCode_;
	uint32_t revisionId_ = 1;
	uint32_t serialNumber_ = 1;
};
}  // namespace modm_canopen
