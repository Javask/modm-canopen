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
	bool polarityInverted{false};
	uint32_t numerator{1};
	uint32_t divisor{1};

	template<typename Integer>
	inline Integer
	toInternal(Integer user)
	{
		static_assert(std::is_integral<Integer>::value);
		typedef typename std::conditional<std::is_signed<Integer>::value, int32_t, uint32_t>::type
			Internal;
		Internal result = (Internal)user * numerator / divisor;

		if constexpr (std::is_signed<Integer>::value)
		{
			if (polarityInverted) result = -result;
		}

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
		typedef typename std::conditional<std::is_signed<Integer>::value, int32_t, uint32_t>::type
			Internal;
		Internal result = (Internal)internal * divisor / numerator;
		if constexpr (std::is_signed<Integer>::value)
		{
			if (polarityInverted) result = -result;
		}
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
	// Position Scaling: Scales: 6062,607A,6067,6068,60F4
	ScalingFactor position{};  // 0x6093.1, 0x6093.2

	// Velocity Scaling: Scales: 606C,606B,606F,60FF,60F8,6081
	ScalingFactor velocity{};  // 0x6094.1, 0x6094.2

	// Acceleration Scaling: Scales: 6083,6085
	ScalingFactor acceleration{};  // 0x6097.1, 0x6097.2

	// Polarity, modifies all. Represents 0x607E
	inline void
	setPolarity(uint8_t polarity)
	{
		position.polarityInverted = (bool)(polarity & (1 << 7));
		velocity.polarityInverted = (bool)(polarity & (1 << 6));
	}
	inline uint8_t
	getPolarity() const
	{
		return (position.polarityInverted ? (1 << 7) : 0) |
			   (velocity.polarityInverted ? (1 << 6) : 0);
	}
};
}  // namespace cia402
}  // namespace modm_canopen