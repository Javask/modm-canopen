#pragma once
#include <cstdint>
namespace modm_canopen
{
namespace cia402
{
// Bits of the cia402 control word (0x6040):
enum class CommandBits : uint16_t
{
	SwitchOn = (1 << 0),
	EnableVoltage = (1 << 1),
	QuickStop = (1 << 2),
	EnableOperation = (1 << 3),
	// 4-6: Mode Specific
	NewSetPoint = (1 << 4),        // ProfilePosition
	ChangeImmediately = (1 << 5),  // ProfilePosition(TODO implement)
	IsRelative = (1 << 6),         // ProfilePosition

	FaultReset = (1 << 7),
	Halt = (1 << 8),  // TODO(implement)
					  // 9-10: Reserved
					  // 11: Manufacturer Specific
					  // 12-15: We dont need these for now
};
}  // namespace cia402
}  // namespace modm_canopen