#pragma once
#include "command.hpp"
#include "states.hpp"
#include <array>

namespace modm_canopen
{
namespace cia402
{
constexpr std::array<Command, 8> StateCommands{
	Command{
		.name = CommandName::Shutdown,
		.bitMask{0b1000'0111},
		.value{0b0000'0110},
		.changing{0b0000'0000},
		.destination = State::ReadyToSwitchOn,
		.availableIn{State::SwitchOnDisabled, State::SwitchedOn, State::Invalid},
	},
	Command{
		.name = CommandName::Shutdown,
		.bitMask{0b1000'0111},
		.value{0b0000'0110},
		.changing{0b0000'0000},
		.destination = State::ShutdownReactionActive,
		.availableIn{State::OperationEnabled, State::Invalid, State::Invalid},
	},
	Command{
		.name = CommandName::SwitchOn,
		.bitMask{0b1000'1111},
		.value{0b0000'0111},
		.changing{0b0000'0000},
		.destination = State::SwitchedOn,
		.availableIn{State::ReadyToSwitchOn, State::Invalid, State::Invalid},
	},
	Command{
		.name = CommandName::DisableVoltage,
		.bitMask{0b1000'0010},
		.value{0b0000'0000},
		.changing{0b0000'0000},
		.destination = State::SwitchOnDisabled,
		.availableIn{State::ReadyToSwitchOn, State::OperationEnabled, State::SwitchedOn},
	},

	Command{
		.name = CommandName::DisableOperation,
		.bitMask{0b1000'1111},
		.value{0b0000'0111},
		.changing{0b0000'0000},
		.destination = State::DisableReactionActive,
		.availableIn{State::OperationEnabled, State::Invalid, State::Invalid},
	},
	Command{
		.name = CommandName::EnableOperation,
		.bitMask{0b1000'1111},
		.value{0b0000'1111},
		.changing{0b0000'0000},
		.destination = State::OperationEnabled,
		.availableIn{State::SwitchedOn, State::QuickStopActive, State::Invalid},
	},
	Command{
		.name = CommandName::FaultReset,
		.bitMask{0b1000'0000},
		.value{0b1000'0000},
		.changing{0b1000'0000},
		.destination = State::SwitchOnDisabled,
		.availableIn{State::Fault, State::Invalid, State::Invalid},
	},
	Command{
		.name = CommandName::QuickStop,
		.bitMask{0b1000'0110},
		.value{0b0000'0010},
		.changing{0b0000'0000},
		.destination = State::QuickStopActive,
		.availableIn{State::OperationEnabled, State::Invalid, State::Invalid},
	},
};
}  // namespace cia402
}  // namespace modm_canopen