#pragma once
#include <cstdint>
#include <type_traits>
#include <limits>
#include <cmath>

#include <modm/debug/logger.hpp>
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

	template<typename Internal, typename User>
	inline Internal
	toInternal(User user) const
	{
		static_assert(std::is_arithmetic_v<Internal> && std::is_arithmetic_v<User>);
		float result =
			static_cast<float>(user) * static_cast<float>(numerator) / static_cast<float>(divisor);

		if constexpr (std::is_integral_v<Internal>) { result = std::round(result); }

		if constexpr (std::is_signed_v<Internal>)
		{
			if (polarityInverted) result = -result;
		}

		if (result > std::numeric_limits<Internal>::max())
		{
			return std::numeric_limits<Internal>::max();
		}
		if (result < std::numeric_limits<Internal>::min())
		{
			return std::numeric_limits<Internal>::min();
		}
		return static_cast<Internal>(result);
	}

	template<typename User, typename Internal>
	inline User
	toUser(Internal internal) const
	{
		static_assert(std::is_arithmetic_v<Internal> && std::is_arithmetic_v<User>);
		float result = static_cast<float>(internal) * static_cast<float>(divisor) /
					   static_cast<float>(numerator);

		if constexpr (std::is_integral_v<User>) { result = std::round(result); }

		if constexpr (std::is_signed_v<User>)
		{
			if (polarityInverted) result = -result;
		}

		if (result > std::numeric_limits<User>::max()) { return std::numeric_limits<User>::max(); }
		if (result < std::numeric_limits<User>::min()) { return std::numeric_limits<User>::min(); }
		return static_cast<User>(result);
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