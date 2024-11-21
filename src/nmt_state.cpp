#include "nmt_state.hpp"

namespace modm_canopen
{
const char*
nmtStateToString(NMTState state)
{
	switch (state)
	{
		case NMTState::BootUp:
			return "BootUp";
		case NMTState::Stopped:
			return "Stopped";
		case NMTState::PreOperational:
			return "PreOperational";
		case NMTState::Operational:
			return "Operational";
		default:
			return "Invalid";
	}
}

std::optional<NMTState>
toNMTState(uint8_t value)
{
	switch (value)
	{
		case (uint8_t)NMTState::BootUp:
			return NMTState::BootUp;
		case (uint8_t)NMTState::Stopped:
			return NMTState::Stopped;
		case (uint8_t)NMTState::PreOperational:
			return NMTState::PreOperational;
		case (uint8_t)NMTState::Operational:
			return NMTState::Operational;
		default:
			return {};
	}
}
}  // namespace modm_canopen