#pragma once
#include <cstdint>
#include <type_traits>
#include <limits>
namespace modm_canopen
{
namespace cia402
{

// Scaling Factor Formula: Internal = User * (Numerator/Divisor)
struct ScalingFactor
{
	uint32_t numerator{1};
	uint32_t divisor{1};

	template<typename Integer>
	inline Integer
	toInternal(Integer user)
	{
		static_assert(std::is_integral<Integer>::value);
		int32_t result = (int32_t)user * numerator / divisor;
		if (result > std::numeric_limits<Integer>::max())
		{
			return std::numeric_limits<Integer>::max();
		}
		if (result < std::numeric_limits<Integer>::min())
		{
			return std::numeric_limits<Integer>::min();
		}
		return (Integer)(result);
	}

	template<typename Integer>
	inline Integer
	toUser(Integer internal)
	{
		static_assert(std::is_integral<Integer>::value);
		int32_t result = (int32_t)internal * divisor / numerator;
		if (result > std::numeric_limits<Integer>::max())
		{
			return std::numeric_limits<Integer>::max();
		}
		if (result < std::numeric_limits<Integer>::min())
		{
			return std::numeric_limits<Integer>::min();
		}
		return (Integer)(result);
	}
};

struct Factors
{
	uint8_t polarity = 0;  // 0x607E

	// Position Scaling: Scales: 6062,607A,6067,6068,60F4
	ScalingFactor position{};  // 0x6093.1, 0x6093.2

	// Velocity Scaling: Scales: 606C,606B,606F,60FF,60F8,6081
	ScalingFactor velocity{};  // 0x6094.1, 0x6094.2

	// Acceleration Scaling: Scales: 6083,6085
	ScalingFactor acceleration{};  // 0x6097.1, 0x6097.2
};
}  // namespace cia402
}  // namespace modm_canopen