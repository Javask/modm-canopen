#pragma once
#include "../object_dictionary_common.hpp"
namespace modm_canopen::cia402
{
struct CiA402Objects
{
	static constexpr modm_canopen::Address ControlWord{0x6040, 0};
	static constexpr modm_canopen::Address StatusWord{0x6041, 0};
	static constexpr modm_canopen::Address ModeOfOperation{0x6060, 0};
	static constexpr modm_canopen::Address ModeOfOperationDisplay{0x6061, 0};
	static constexpr modm_canopen::Address SupportedDriveModes{0x6502, 0};
};
}