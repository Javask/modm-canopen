#pragma once
#include <cstdint>
#include <cmath>

namespace modm_canopen::cia402
{
class Trajectory
{
private:
	uint32_t currentTime_{0};
	int32_t currentVel_{0};
	int32_t currentPos_{0};

	int32_t startVel_{0};
	int32_t startPos_{0};

	int32_t outPos_{0};
	int32_t outVel_{0};
	int32_t outAccel_{0};

    int32_t targetPos_{0};

    uint32_t profileJerk_{0};
    uint32_t profileAcceleration_{0};
    uint32_t profileVelocity_{0};

	uint32_t maxJerk_{0};
	uint32_t maxAcceleration_{0};
	uint32_t maxVelocity_{0};
	int32_t maxPosition_{1000};
	int32_t minPosition_{-1000};

	static inline bool
	isBetween(int32_t val, int32_t limA, int32_t limB)
	{
		const auto minV = std::min(limA, limB);
		const auto maxV = std::max(limA, limB);
		return minV <= val && val <= maxV;
	}

public:
	Trajectory() = default;
	~Trajectory() = default;

	void
	update(uint32_t timestep);
};

}  // namespace modm_canopen::cia402