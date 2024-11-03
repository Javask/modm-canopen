#pragma once
#include <cstdint>
#include <type_traits>
#include <limits>
#include <cmath>
#include "scaling_objects.hpp"

namespace modm_canopen
{
namespace cia402
{

// Scaling Factor Formula: Internal = User * (Numerator/Divisor)
struct ScalingFactor
{
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

		if (result > std::numeric_limits<User>::max()) { return std::numeric_limits<User>::max(); }
		if (result < std::numeric_limits<User>::min()) { return std::numeric_limits<User>::min(); }
		return static_cast<User>(result);
	}
};

class CiA402Factors
{
public:
	static inline ScalingFactor positionEncoderResolution{};  // 0x608F.1, 0x608F.2
	static inline ScalingFactor velocityEncoderResolution{};  // 0x6090.1, 0x6090.2
	static inline ScalingFactor gearRatio{};                  // 0x6091.1, 0x6091.2
	static inline ScalingFactor feed{};                       // 0x6092.1, 0x6092.2

	// Position Scaling: Scales: 6062,607A,6067,6068,60F4
	static inline ScalingFactor position{};  // 0x6093.1, 0x6093.2

	// Velocity Scaling: Scales: 606C,606B,606F,60FF,60F8,6081
	static inline ScalingFactor velocityEncoder{};  // 0x6094.1, 0x6094.2
	static inline ScalingFactor velocity1{};        // 0x6095.1, 0x6095.2
	static inline ScalingFactor velocity2{};        // 0x6096.1, 0x6096.2

	// Acceleration Scaling: Scales: 6083,6085
	static inline ScalingFactor acceleration{};  // 0x6097.1, 0x6097.2

	// Polarity, modifies all. Represents 0x607E
	static inline bool velocityInverted = false;
	static inline bool positionInverted = false;
	static inline void
	setPolarity(uint8_t polarity)
	{
		positionInverted = (bool)(polarity & (1 << 7));
		velocityInverted = (bool)(polarity & (1 << 6));
	}
	static inline uint8_t
	getPolarity()
	{
		return (positionInverted ? (1 << 7) : 0) | (velocityInverted ? (1 << 6) : 0);
	}
protected:
friend class CiA402;

	template<typename OD>
	constexpr static inline void
	registerHandlers(HandlerMap<OD>& map)
	{
		map.template setReadHandler<ScalingObjects::PositionEncoderResolutionNumerator>(
			+[]() { return positionEncoderResolution.numerator; });

		map.template setWriteHandler<ScalingObjects::PositionEncoderResolutionNumerator>(
			+[](uint32_t value) {
				positionEncoderResolution.numerator = value;
				return SdoErrorCode::NoError;
			});

		map.template setReadHandler<ScalingObjects::PositionEncoderResolutionDivisor>(
			+[]() { return positionEncoderResolution.divisor; });

		map.template setWriteHandler<ScalingObjects::PositionEncoderResolutionDivisor>(
			+[](uint32_t value) {
				positionEncoderResolution.divisor = value;
				return SdoErrorCode::NoError;
			});

		map.template setReadHandler<ScalingObjects::VelocityEncoderResolutionNumerator>(
			+[]() { return velocityEncoderResolution.numerator; });

		map.template setWriteHandler<ScalingObjects::VelocityEncoderResolutionNumerator>(
			+[](uint32_t value) {
				velocityEncoderResolution.numerator = value;
				return SdoErrorCode::NoError;
			});

		map.template setReadHandler<ScalingObjects::VelocityEncoderResolutionDivisor>(
			+[]() { return velocityEncoderResolution.divisor; });

		map.template setWriteHandler<ScalingObjects::VelocityEncoderResolutionDivisor>(
			+[](uint32_t value) {
				velocityEncoderResolution.divisor = value;
				return SdoErrorCode::NoError;
			});

		map.template setReadHandler<ScalingObjects::GearRatioNumerator>(
			+[]() { return gearRatio.numerator; });

		map.template setWriteHandler<ScalingObjects::GearRatioNumerator>(+[](uint32_t value) {
			gearRatio.numerator = value;
			return SdoErrorCode::NoError;
		});

		map.template setReadHandler<ScalingObjects::GearRatioDivisor>(
			+[]() { return gearRatio.divisor; });

		map.template setWriteHandler<ScalingObjects::GearRatioDivisor>(+[](uint32_t value) {
			gearRatio.divisor = value;
			return SdoErrorCode::NoError;
		});

		map.template setReadHandler<ScalingObjects::FeedNumerator>(
			+[]() { return feed.numerator; });

		map.template setWriteHandler<ScalingObjects::FeedNumerator>(+[](uint32_t value) {
			feed.numerator = value;
			return SdoErrorCode::NoError;
		});

		map.template setReadHandler<ScalingObjects::FeedDivisor>(+[]() { return feed.divisor; });

		map.template setWriteHandler<ScalingObjects::FeedDivisor>(+[](uint32_t value) {
			feed.divisor = value;
			return SdoErrorCode::NoError;
		});

		map.template setReadHandler<ScalingObjects::PositionFactorNumerator>(
			+[]() { return position.numerator; });

		map.template setWriteHandler<ScalingObjects::PositionFactorNumerator>(+[](uint32_t value) {
			position.numerator = value;
			return SdoErrorCode::NoError;
		});

		map.template setReadHandler<ScalingObjects::PositionFactorDivisor>(
			+[]() { return position.divisor; });

		map.template setWriteHandler<ScalingObjects::PositionFactorDivisor>(+[](uint32_t value) {
			position.divisor = value;
			return SdoErrorCode::NoError;
		});

		map.template setReadHandler<ScalingObjects::VelocityEncoderFactorNumerator>(
			+[]() { return velocityEncoder.numerator; });

		map.template setWriteHandler<ScalingObjects::VelocityEncoderFactorNumerator>(
			+[](uint32_t value) {
				velocityEncoder.numerator = value;
				return SdoErrorCode::NoError;
			});

		map.template setReadHandler<ScalingObjects::VelocityEncoderFactorDivisor>(
			+[]() { return velocityEncoder.divisor; });

		map.template setWriteHandler<ScalingObjects::VelocityEncoderFactorDivisor>(
			+[](uint32_t value) {
				velocityEncoder.divisor = value;
				return SdoErrorCode::NoError;
			});

		map.template setReadHandler<ScalingObjects::VelocityFactor1Numerator>(
			+[]() { return velocity1.numerator; });

		map.template setWriteHandler<ScalingObjects::VelocityFactor1Numerator>(+[](uint32_t value) {
			velocity1.numerator = value;
			return SdoErrorCode::NoError;
		});

		map.template setReadHandler<ScalingObjects::VelocityFactor1Divisor>(
			+[]() { return velocity1.divisor; });

		map.template setWriteHandler<ScalingObjects::VelocityFactor1Divisor>(+[](uint32_t value) {
			velocity1.divisor = value;
			return SdoErrorCode::NoError;
		});

		map.template setReadHandler<ScalingObjects::VelocityFactor2Numerator>(
			+[]() { return velocity2.numerator; });

		map.template setWriteHandler<ScalingObjects::VelocityFactor2Numerator>(+[](uint32_t value) {
			velocity2.numerator = value;
			return SdoErrorCode::NoError;
		});

		map.template setReadHandler<ScalingObjects::VelocityFactor2Divisor>(
			+[]() { return velocity2.divisor; });

		map.template setWriteHandler<ScalingObjects::VelocityFactor2Divisor>(+[](uint32_t value) {
			velocity2.divisor = value;
			return SdoErrorCode::NoError;
		});

		map.template setReadHandler<ScalingObjects::AccelerationFactorNumerator>(
			+[]() { return acceleration.numerator; });

		map.template setWriteHandler<ScalingObjects::AccelerationFactorNumerator>(
			+[](uint32_t value) {
				acceleration.numerator = value;
				return SdoErrorCode::NoError;
			});

		map.template setReadHandler<ScalingObjects::AccelerationFactorDivisor>(
			+[]() { return acceleration.divisor; });

		map.template setWriteHandler<ScalingObjects::AccelerationFactorDivisor>(
			+[](uint32_t value) {
				acceleration.divisor = value;
				return SdoErrorCode::NoError;
			});

		map.template setReadHandler<ScalingObjects::Polarity>(+[]() { return getPolarity(); });

		map.template setWriteHandler<ScalingObjects::Polarity>(+[](uint8_t value) {
			setPolarity(value);
			return SdoErrorCode::NoError;
		});
	}
};
}  // namespace cia402
}  // namespace modm_canopen