#pragma once
#include <cstdint>
namespace modm_canopen::cia402
{
enum class ProfileType : int16_t
{
	LinearRamp = 0,
	// Sin2Ramp = 1
};
}