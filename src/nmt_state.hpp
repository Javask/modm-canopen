#pragma once
#include <cstdint>
namespace modm_canopen
{
enum class NMTState : uint8_t
{
	// Initialisation - Unsupported
	Stopped = 4,
	PreOperational = 127,
	Operational = 5,
};

const char*
nmtStateToString(NMTState state);
}  // namespace modm_canopen