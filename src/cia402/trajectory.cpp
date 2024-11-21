#include "trajectory.hpp"
#include <algorithm>
using namespace modm_canopen::cia402;

void
Trajectory::update(uint32_t timeStep)
{
	// Clamp to limits
	const auto limitedAccel = std::clamp(profileAcceleration_, 0u, maxAcceleration_);
	const auto limitedVel = std::clamp(profileVelocity_, -maxVelocity_, maxVelocity_);
	const auto limitedTargetPos = std::clamp(targetPos_, minPosition_, maxPosition_);

	// This is currently a trapezoidal position step
	// Check if we are even going in the right direction
	auto shouldBrake =
		std::signbit(limitedTargetPos - startPos_) != std::signbit(currentVel_) && currentVel_ != 0;

	if (!shouldBrake)
	{
		// If we are check if we have to brake at the desired decel to make it
		const auto timeToBrake =
			(std::round(std::abs((float)currentVel_ / (limitedAccel * timeStep))) * timeStep);
		const auto posAfterBraking = (currentPos_ + currentVel_ * timeToBrake +
									  (0.5 * -(limitedAccel * timeToBrake * timeToBrake)) *
										  (std::signbit(currentVel_) ? 1 : -1));

		// Depending on movement direction brake if we would otherwise overshoot
		if (limitedTargetPos >= startPos_)
		{
			shouldBrake = posAfterBraking >= limitedTargetPos;
		} else
		{
			shouldBrake = posAfterBraking <= limitedTargetPos;
		}
	}

	// Select the velocity we actually want to reach
	int32_t targetVel = targetPos_ < startPos_ ? -limitedVel : limitedVel;

	if (shouldBrake)
	{
		// If we want to brake, accelerate against our current velocity
		outAccel_ = (std::signbit(currentVel_) ? limitedAccel : -limitedAccel);
		targetVel = 0;
	} else if (currentVel_ > targetVel)
	{
		// If velocity is too high, decelerate
		outAccel_ = -limitedAccel;
	} else if (currentVel_ < targetVel)
	{
		// If velocity is too low, acccelerate
		outAccel_ = limitedAccel;
	} else
	{
		// We are at the correct velocity
		outAccel_ = 0;
	}

	// Intergrate desired output velocity
	outVel_ = currentVel_ + outAccel_ * timeStep;
	if (std::abs(outVel_) > (int32_t)maxVelocity_)
	{
		// Remove overshoot due to maximum
		outAccel_ -= (outVel_ - std::copysign(maxVelocity_, outVel_)) / timeStep;
		outVel_ = currentVel_ + outAccel_ * timeStep;
	}
	if (isBetween(targetVel, outVel_, currentVel_))
	{
		// Remove overshoot due to timesteps
		outAccel_ -= (outVel_ - targetVel) / timeStep;
		outVel_ = currentVel_ + outAccel_ * timeStep;
	}
	// Integrate desired output velocity
	outPos_ = currentPos_ + outVel_ * timeStep;
	// If we remove overshoot here we generate really high accelerations
}