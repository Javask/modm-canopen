#include "nmt_state.hpp"

namespace modm_canopen
{
const char*
nmtStateToString(NMTState state)
{
	switch (state)
	{
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
}  // namespace modm_canopen