#pragma once
#include "command.hpp"
#include "states.hpp"
#include <array>

namespace modm_canopen
{
namespace cia402
{
struct StateCommand
{
	Command cmd;
	State destination;
	State availableIn[3];
};

constexpr std::array<StateCommand, 7> StateCommands{
	StateCommand{
		.cmd =
			Command{
				.bitMask{0b1000'0111},
				.value{0b0000'0110},
				.changing{0b0000'0000},
			},
		.destination = State::ReadyToSwitchOn,  // Shutdown
		.availableIn{State::SwitchOnDisabled, State::SwitchedOn, State::OperationEnabled},
	},
	StateCommand{
		.cmd =
			Command{
				.bitMask{0b1000'1111},
				.value{0b0000'0111},
				.changing{0b0000'0000},
			},
		.destination = State::SwitchedOn,  // SwitchOn
		.availableIn{State::ReadyToSwitchOn, State::Invalid, State::Invalid},
	},
	StateCommand{
		.cmd =
			Command{
				.bitMask{0b1000'0010},
				.value{0b0000'0000},
				.changing{0b0000'0000},
			},
		.destination = State::SwitchOnDisabled,  // DisableVoltage
		.availableIn{State::ReadyToSwitchOn, State::OperationEnabled, State::SwitchedOn},
	},
	StateCommand{
		.cmd =
			Command{
				.bitMask{0b1000'1111},
				.value{0b0000'0111},
				.changing{0b0000'0000},
			},
		.destination = State::SwitchedOn,  // DisableOperation
		.availableIn{State::OperationEnabled, State::Invalid, State::Invalid},
	},
	StateCommand{
		.cmd =
			Command{
				.bitMask{0b1000'1111},
				.value{0b0000'1111},
				.changing{0b0000'0000},
			},
		.destination = State::OperationEnabled,  // EnableOperation
		.availableIn{State::SwitchedOn, State::QuickStopActive, State::Invalid},
	},
	StateCommand{
		.cmd =
			Command{
				.bitMask{0b1000'0000},
				.value{0b1000'0000},
				.changing{0b1000'0000},
			},
		.destination = State::SwitchOnDisabled,  // FaultReset
		.availableIn{State::Fault, State::Invalid, State::Invalid},
	},
	StateCommand{
		.cmd =
			Command{
				.bitMask{0b1000'0110},
				.value{0b0000'0010},
				.changing{0b0000'0000},
			},
		.destination = State::QuickStopActive,  // QuickStop
		.availableIn{State::OperationEnabled, State::Invalid, State::Invalid},
	},
};

enum class StateCommandNames : uint8_t
{
	Shutdown = 0,
	SwitchOn = 1,
	DisableVoltage = 2,
	DisableOperation = 3,
	EnableOperation = 4,
	FaultReset = 5,
};

}  // namespace cia402
}  // namespace modm_canopen