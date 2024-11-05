#pragma once
#include "../object_dictionary_common.hpp"
namespace modm_canopen::cia402
{
template<uint8_t Axis>
struct CiA402Objects
{
	static constexpr modm_canopen::Address ControlWord{0x6040 + 0x800 * Axis, 0};
	static constexpr modm_canopen::Address StatusWord{0x6041 + 0x800 * Axis, 0};
	static constexpr modm_canopen::Address ModeOfOperation{0x6060 + 0x800 * Axis, 0};
	static constexpr modm_canopen::Address ModeOfOperationDisplay{0x6061 + 0x800 * Axis, 0};
	static constexpr modm_canopen::Address SupportedDriveModes{0x6502 + 0x800 * Axis, 0};

	static constexpr modm_canopen::Address QuickStopOptionCode{0x605A + 0x800 * Axis, 0};
	static constexpr modm_canopen::Address ShutdownOptionCode{0x605B + 0x800 * Axis, 0};
	static constexpr modm_canopen::Address DisableOperationOptionCode{0x605C + 0x800 * Axis, 0};
	static constexpr modm_canopen::Address HaltOptionCode{0x605D + 0x800 * Axis, 0};
	static constexpr modm_canopen::Address FaultReactionOptionCode{0x605E + 0x800 * Axis, 0};
};
}  // namespace modm_canopen::cia402