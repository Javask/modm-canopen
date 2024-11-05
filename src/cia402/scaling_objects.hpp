#pragma once
#include "../object_dictionary_common.hpp"
namespace modm_canopen::cia402
{
template<uint8_t Axis>
struct ScalingObjects
{
	static constexpr modm_canopen::Address PositionEncoderResolutionNumerator{0x608F + 0x800 * Axis,
																			  1};
	static constexpr modm_canopen::Address PositionEncoderResolutionDivisor{0x608F + 0x800 * Axis,
																			2};
	static constexpr modm_canopen::Address VelocityEncoderResolutionNumerator{0x6090 + 0x800 * Axis,
																			  1};
	static constexpr modm_canopen::Address VelocityEncoderResolutionDivisor{0x6090 + 0x800 * Axis,
																			2};
	static constexpr modm_canopen::Address GearRatioNumerator{0x6091 + 0x800 * Axis, 1};
	static constexpr modm_canopen::Address GearRatioDivisor{0x6091 + 0x800 * Axis, 2};
	static constexpr modm_canopen::Address FeedNumerator{0x6092 + 0x800 * Axis, 1};
	static constexpr modm_canopen::Address FeedDivisor{0x6092 + 0x800 * Axis, 2};
	static constexpr modm_canopen::Address PositionFactorNumerator{0x6093 + 0x800 * Axis, 1};
	static constexpr modm_canopen::Address PositionFactorDivisor{0x6093 + 0x800 * Axis, 2};
	static constexpr modm_canopen::Address VelocityEncoderFactorNumerator{0x6094 + 0x800 * Axis, 1};
	static constexpr modm_canopen::Address VelocityEncoderFactorDivisor{0x6094 + 0x800 * Axis, 2};
	static constexpr modm_canopen::Address VelocityFactor1Numerator{0x6095 + 0x800 * Axis, 1};
	static constexpr modm_canopen::Address VelocityFactor1Divisor{0x6095 + 0x800 * Axis, 2};
	static constexpr modm_canopen::Address VelocityFactor2Numerator{0x6096 + 0x800 * Axis, 1};
	static constexpr modm_canopen::Address VelocityFactor2Divisor{0x6096 + 0x800 * Axis, 2};
	static constexpr modm_canopen::Address AccelerationFactorNumerator{0x6097 + 0x800 * Axis, 1};
	static constexpr modm_canopen::Address AccelerationFactorDivisor{0x6097 + 0x800 * Axis, 2};
	static constexpr modm_canopen::Address Polarity{0x607E + 0x800 * Axis, 0};
};
}  // namespace modm_canopen::cia402