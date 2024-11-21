#ifndef CANOPEN_HEARTBEAT_HPP
#define CANOPEN_HEARTBEAT_HPP
#include <modm/architecture/interface/can_message.hpp>
#include <cstdint>
#include <modm/processing/timer.hpp>
#include <modm/debug/logger.hpp>

using namespace std::chrono_literals;

namespace modm_canopen
{

namespace detail
{

void
makeHeartbeatMSG(uint8_t canId, modm::can::Message &message, uint8_t mode)
{
	message = modm::can::Message{0x700 + (uint32_t)canId, 1};
	message.setExtended(false);
	message.data[0] = mode;
}
}  // namespace detail

template<typename Device>
class Heartbeat
{

private:
	static inline bool heartbeatMissed_{false};
	static inline bool firstUpdate_{true};
	static inline modm::PreciseClock::time_point lastUpdate_{};

	static inline modm::PreciseClock::duration heartbeatProducerTime_{0ms};

	static inline uint8_t expectedHeartbeatNodeId_{0};
	static inline modm::PreciseClock::duration heartbeatConsumerTime_{0ms};
	static inline modm::PreciseClock::time_point lastHeartbeatTime_{};

public:
	template<typename MessageCallback>
	static void
	update(MessageCallback &&cb)
	{
		auto now = modm::PreciseClock::now();
		// Bootup service
		if (firstUpdate_)
		{
			// Technically not heartbeat but it uses the same id
			firstUpdate_ = false;
			lastUpdate_ = now;
			modm::can::Message msg{};
			detail::makeHeartbeatMSG(Device::nodeId(), msg, 0x0);  // Boot up
			cb(msg);
		} else
		{
			if (heartbeatProducerTime_ != 0ms && (now - lastUpdate_) > heartbeatProducerTime_)
			{
				lastUpdate_ = now;
				modm::can::Message msg{};
				detail::makeHeartbeatMSG(Device::nodeId(), msg, (uint8_t)Device::nmtState());
				cb(msg);
			}
		}

		if (heartbeatConsumerTime_ != 0ms && lastHeartbeatTime_.time_since_epoch().count() != 0)
		{
			if (now - lastHeartbeatTime_ > heartbeatConsumerTime_)
			{
				Device::setError(EMCYError::HeartbeartOrGuardingError);
				heartbeatMissed_ = true;
			}
		}
	}

	template<typename MessageCallback>
	static void
	processMessage(const modm::can::Message &message, MessageCallback &&cb)
	{
		// Node guarding
		if (message.getIdentifier() == 0x700u + Device::nodeId() &&
			message.isRemoteTransmitRequest())
		{
			// Technically not heartbeat but it uses the same id
			lastUpdate_ = modm::PreciseClock::now();
			modm::can::Message msg{};
			detail::makeHeartbeatMSG(Device::nodeId(), msg, (uint8_t)Device::nmtState());
			cb(msg);
		}

		if (message.getIdentifier() == 0x700u + expectedHeartbeatNodeId_)
		{
			if (message.getLength() != 1)
			{
				// Invalid length!
				Device::setError(EMCYError::HeartbeartOrGuardingError);
			} else
			{
				lastHeartbeatTime_ = modm::PreciseClock::now();
				heartbeatMissed_ = false;
			}
		}
	}

	constexpr void
	registerHandlers(Device::Map &map)
	{
		using modm_canopen::SdoErrorCode;
		map.template setReadHandler<Address{0x1017, 0}>(
			+[]() { return (uint16_t)(heartbeatProducerTime_.count() / 1000); });

		map.template setWriteHandler<Address{0x1017, 0}>(+[](uint16_t value) {
			heartbeatProducerTime_ = std::chrono::milliseconds((int)value);
			return SdoErrorCode::NoError;
		});

		map.template setReadHandler<Address{0x1016, 0}>(+[]() { return (uint32_t)1; });
		map.template setReadHandler<Address{0x1016, 1}>(+[]() {
			return (((uint32_t)heartbeatConsumerTime_.count()) & 0xFFFF) |
				   (((uint32_t)expectedHeartbeatNodeId_) << 16);
		});

		map.template setWriteHandler<Address{0x1016, 1}>(+[](uint32_t value) {
			heartbeatConsumerTime_ = std::chrono::milliseconds(value & 0xFFFF);
			expectedHeartbeatNodeId_ = (value >> 16) & 0xFF;
			return SdoErrorCode::NoError;
		});
	}

	static bool
	hasMissedHeartbeat()
	{
		return heartbeatMissed_;
	}

	static void
	resetHeartbeatMissed()
	{
		heartbeatMissed_ = false;
	}
};

}  // namespace modm_canopen
#endif