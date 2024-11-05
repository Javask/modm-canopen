#ifndef CANOPEN_FACTORS_HPP
#error "Do not include this file directly, use factors.hpp instead"
#endif

namespace modm_canopen::cia402
{

template<typename Internal, typename User>
Internal
ScalingFactor::toInternal(User user) const
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
User
ScalingFactor::toUser(Internal internal) const
{
	static_assert(std::is_arithmetic_v<Internal> && std::is_arithmetic_v<User>);
	float result =
		static_cast<float>(internal) * static_cast<float>(divisor) / static_cast<float>(numerator);

	if constexpr (std::is_integral_v<User>) { result = std::round(result); }

	if (result > std::numeric_limits<User>::max()) { return std::numeric_limits<User>::max(); }
	if (result < std::numeric_limits<User>::min()) { return std::numeric_limits<User>::min(); }
	return static_cast<User>(result);
}

template<uint8_t Axis>
void
CiA402Factors<Axis>::setPolarity(uint8_t polarity)
{
	positionInverted = (bool)(polarity & (1 << 7));
	velocityInverted = (bool)(polarity & (1 << 6));
}

template<uint8_t Axis>
uint8_t
CiA402Factors<Axis>::getPolarity()
{
	return (positionInverted ? (1 << 7) : 0) | (velocityInverted ? (1 << 6) : 0);
}

template<uint8_t Axis>
template<typename OD>
constexpr void
CiA402Factors<Axis>::registerHandlers(HandlerMap<OD>& map)
{
	map.template setReadHandler<ScalingObjects<Axis>::PositionEncoderResolutionNumerator>(
		+[]() { return positionEncoderResolution.numerator; });

	map.template setWriteHandler<ScalingObjects<Axis>::PositionEncoderResolutionNumerator>(
		+[](uint32_t value) {
			positionEncoderResolution.numerator = value;
			return SdoErrorCode::NoError;
		});

	map.template setReadHandler<ScalingObjects<Axis>::PositionEncoderResolutionDivisor>(
		+[]() { return positionEncoderResolution.divisor; });

	map.template setWriteHandler<ScalingObjects<Axis>::PositionEncoderResolutionDivisor>(
		+[](uint32_t value) {
			positionEncoderResolution.divisor = value;
			return SdoErrorCode::NoError;
		});

	map.template setReadHandler<ScalingObjects<Axis>::VelocityEncoderResolutionNumerator>(
		+[]() { return velocityEncoderResolution.numerator; });

	map.template setWriteHandler<ScalingObjects<Axis>::VelocityEncoderResolutionNumerator>(
		+[](uint32_t value) {
			velocityEncoderResolution.numerator = value;
			return SdoErrorCode::NoError;
		});

	map.template setReadHandler<ScalingObjects<Axis>::VelocityEncoderResolutionDivisor>(
		+[]() { return velocityEncoderResolution.divisor; });

	map.template setWriteHandler<ScalingObjects<Axis>::VelocityEncoderResolutionDivisor>(
		+[](uint32_t value) {
			velocityEncoderResolution.divisor = value;
			return SdoErrorCode::NoError;
		});

	map.template setReadHandler<ScalingObjects<Axis>::GearRatioNumerator>(
		+[]() { return gearRatio.numerator; });

	map.template setWriteHandler<ScalingObjects<Axis>::GearRatioNumerator>(+[](uint32_t value) {
		gearRatio.numerator = value;
		return SdoErrorCode::NoError;
	});

	map.template setReadHandler<ScalingObjects<Axis>::GearRatioDivisor>(
		+[]() { return gearRatio.divisor; });

	map.template setWriteHandler<ScalingObjects<Axis>::GearRatioDivisor>(+[](uint32_t value) {
		gearRatio.divisor = value;
		return SdoErrorCode::NoError;
	});

	map.template setReadHandler<ScalingObjects<Axis>::FeedNumerator>(
		+[]() { return feed.numerator; });

	map.template setWriteHandler<ScalingObjects<Axis>::FeedNumerator>(+[](uint32_t value) {
		feed.numerator = value;
		return SdoErrorCode::NoError;
	});

	map.template setReadHandler<ScalingObjects<Axis>::FeedDivisor>(+[]() { return feed.divisor; });

	map.template setWriteHandler<ScalingObjects<Axis>::FeedDivisor>(+[](uint32_t value) {
		feed.divisor = value;
		return SdoErrorCode::NoError;
	});

	map.template setReadHandler<ScalingObjects<Axis>::PositionFactorNumerator>(
		+[]() { return position.numerator; });

	map.template setWriteHandler<ScalingObjects<Axis>::PositionFactorNumerator>(
		+[](uint32_t value) {
			position.numerator = value;
			return SdoErrorCode::NoError;
		});

	map.template setReadHandler<ScalingObjects<Axis>::PositionFactorDivisor>(
		+[]() { return position.divisor; });

	map.template setWriteHandler<ScalingObjects<Axis>::PositionFactorDivisor>(+[](uint32_t value) {
		position.divisor = value;
		return SdoErrorCode::NoError;
	});

	map.template setReadHandler<ScalingObjects<Axis>::VelocityEncoderFactorNumerator>(
		+[]() { return velocityEncoder.numerator; });

	map.template setWriteHandler<ScalingObjects<Axis>::VelocityEncoderFactorNumerator>(
		+[](uint32_t value) {
			velocityEncoder.numerator = value;
			return SdoErrorCode::NoError;
		});

	map.template setReadHandler<ScalingObjects<Axis>::VelocityEncoderFactorDivisor>(
		+[]() { return velocityEncoder.divisor; });

	map.template setWriteHandler<ScalingObjects<Axis>::VelocityEncoderFactorDivisor>(
		+[](uint32_t value) {
			velocityEncoder.divisor = value;
			return SdoErrorCode::NoError;
		});

	map.template setReadHandler<ScalingObjects<Axis>::VelocityFactor1Numerator>(
		+[]() { return velocity1.numerator; });

	map.template setWriteHandler<ScalingObjects<Axis>::VelocityFactor1Numerator>(
		+[](uint32_t value) {
			velocity1.numerator = value;
			return SdoErrorCode::NoError;
		});

	map.template setReadHandler<ScalingObjects<Axis>::VelocityFactor1Divisor>(
		+[]() { return velocity1.divisor; });

	map.template setWriteHandler<ScalingObjects<Axis>::VelocityFactor1Divisor>(+[](uint32_t value) {
		velocity1.divisor = value;
		return SdoErrorCode::NoError;
	});

	map.template setReadHandler<ScalingObjects<Axis>::VelocityFactor2Numerator>(
		+[]() { return velocity2.numerator; });

	map.template setWriteHandler<ScalingObjects<Axis>::VelocityFactor2Numerator>(
		+[](uint32_t value) {
			velocity2.numerator = value;
			return SdoErrorCode::NoError;
		});

	map.template setReadHandler<ScalingObjects<Axis>::VelocityFactor2Divisor>(
		+[]() { return velocity2.divisor; });

	map.template setWriteHandler<ScalingObjects<Axis>::VelocityFactor2Divisor>(+[](uint32_t value) {
		velocity2.divisor = value;
		return SdoErrorCode::NoError;
	});

	map.template setReadHandler<ScalingObjects<Axis>::AccelerationFactorNumerator>(
		+[]() { return acceleration.numerator; });

	map.template setWriteHandler<ScalingObjects<Axis>::AccelerationFactorNumerator>(
		+[](uint32_t value) {
			acceleration.numerator = value;
			return SdoErrorCode::NoError;
		});

	map.template setReadHandler<ScalingObjects<Axis>::AccelerationFactorDivisor>(
		+[]() { return acceleration.divisor; });

	map.template setWriteHandler<ScalingObjects<Axis>::AccelerationFactorDivisor>(
		+[](uint32_t value) {
			acceleration.divisor = value;
			return SdoErrorCode::NoError;
		});

	map.template setReadHandler<ScalingObjects<Axis>::Polarity>(+[]() { return getPolarity(); });

	map.template setWriteHandler<ScalingObjects<Axis>::Polarity>(+[](uint8_t value) {
		setPolarity(value);
		return SdoErrorCode::NoError;
	});
}
}  // namespace modm_canopen::cia402