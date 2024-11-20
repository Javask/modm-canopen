#pragma once
#include <cstdint>
#include <optional>

namespace modm_canopen
{
enum class NMTState : uint8_t
{
	// Initialisation - Unsupported
	Stopped = 4,
	PreOperational = 127,
	Operational = 5,
};


std::optional<NMTState> 
toNMTState(uint8_t value);

const char*
nmtStateToString(NMTState state);
}  // namespace modm_canopen