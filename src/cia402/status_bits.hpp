#pragma once
#include <cstdint>
namespace modm_canopen
{
namespace cia402
{

// Bits of the cia402 status word (0x6041):
enum class StatusBits : uint16_t
{
	ReadyToSwitchOn = (1 << 0),
	SwitchedOn = (1 << 1),
	OperationEnabled = (1 << 2),
	Fault = (1 << 3),
	VoltagePresent = (1 << 4),
	NotCurrentlyQuickStopping = (1 << 5),
	SwitchOnDisabled = (1 << 6),
	Warning = (1 << 7),
	Busy = (1 << 8),
	Remote = (1 << 9),
	TargetReached = (1 << 10),
	InternalLimitSet = (1 << 11),

	// 12-13: Mode Specific
	SpeedZero = (1 << 12),                 // Profile Velocity Mode
	MaxSlippageError = (1 << 13),          // Profile Velocity Mode
	SetPointAcknowledge = (1 << 12),       // Profile Position Mode
	MaxFollowingError = (1 << 13),         // Profile Position Mode
	HomingAttained = (1 << 12),            // Homing Mode
	HomingError = (1 << 13),               // Homing Mode
	InterpolationModeActiove = (1 << 12),  // Interpolation Position Mode

	// 14-15: Manufacturer defined
};

}  // namespace cia402
}  // namespace modm_canopen