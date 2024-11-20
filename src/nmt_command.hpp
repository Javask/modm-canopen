#pragma once
#include <cstdint>
namespace modm_canopen
{
enum class NMTCommand : uint8_t
{
	Start = 1,
	Stop = 2,
	EnterPreOperational = 128,
	ResetNode = 129
};

static inline bool
isValidNMTCommand(uint8_t val)
{
	return val == 1 || val == 2 || val == 128 || val == 129;
}
}  // namespace modm_canopen