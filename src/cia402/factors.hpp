#ifndef CANOPEN_FACTORS_HPP
#define CANOPEN_FACTORS_HPP
#include <cstdint>
#include <type_traits>
#include <limits>
#include <cmath>
#include "scaling_objects.hpp"

namespace modm_canopen::cia402
{

// Scaling Factor Formula: Internal = User * (Numerator/Divisor)
struct ScalingFactor
{
	uint32_t numerator{1};
	uint32_t divisor{1};

	template<typename Internal, typename User>
	inline Internal
	toInternal(User user) const;

	template<typename User, typename Internal>
	inline User
	toUser(Internal internal) const;
};

template<uint8_t Axis>
class CiA402;

template<uint8_t Axis>
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
	setPolarity(uint8_t polarity);
	static inline uint8_t
	getPolarity();

protected:
	friend class CiA402<Axis>;

	template<typename OD>
	constexpr static inline void
	registerHandlers(HandlerMap<OD>& map);
};
}  // namespace modm_canopen::cia402

#include "factors_impl.hpp"
#endif