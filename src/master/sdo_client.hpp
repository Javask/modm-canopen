#ifndef CANOPEN_SDO_CLIENT_HPP
#define CANOPEN_SDO_CLIENT_HPP
#include <modm/architecture/interface/can_message.hpp>
#include "../object_dictionary.hpp"
#include <future>
#include <vector>
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <modm/processing/timer.hpp>

namespace modm_canopen
{
// Needs a heap allocator, could be changed, but im only planning on using it on a linux host
template<typename Device>
class SdoClient
{
public:
	template<typename MessageCallback>
	static void
	requestRead(uint8_t canId, Address address, MessageCallback&& sendMessage);

	template<typename MessageCallback>
	static void
	requestRead(uint8_t canId, Address address,
				std::function<void(const uint8_t, Value)>&& valueCallback,
				MessageCallback&& sendMessage);

	template<typename MessageCallback>
	static void
	requestWrite(uint8_t canId, Address address, MessageCallback&& sendMessage);

	template<typename MessageCallback>
	static void
	requestWrite(uint8_t canId, Address address, const Value& value, MessageCallback&& sendMessage);

	template<typename MessageCallback>
	static void
	processMessage(const modm::can::Message& request, MessageCallback&& responseCallback);

	template<typename MessageCallback>
	static void
	update(MessageCallback&& sendMessage);

	static bool
	waiting();

	static bool
	waitingOn(uint8_t id);

private:
	struct WaitingEntry
	{

		uint8_t canId;
		Address address;
		bool isRead;
		modm::Clock::time_point sent;
		modm::can::Message msg;
		std::function<void(const uint8_t, Value)> callback;

		inline WaitingEntry()
		{
			canId = 0;
			address = {};
			isRead = false;
			sent = {};
			msg = {};
			callback = {};
		}

		inline WaitingEntry&
		operator=(const WaitingEntry& other)
		{
			canId = other.canId;
			address = other.address;
			isRead = other.isRead;
			sent = other.sent;
			msg = other.msg;
			callback = other.callback;
			return *this;
		}

		inline WaitingEntry(const WaitingEntry& other)
		{
			canId = other.canId;
			address = other.address;
			isRead = other.isRead;
			sent = other.sent;
			msg = other.msg;
			callback = other.callback;
		};

		inline WaitingEntry&
		operator=(WaitingEntry&& other)
		{
			canId = std::move(other.canId);
			address = std::move(other.address);
			isRead = std::move(other.isRead);
			sent = std::move(other.sent);
			msg = std::move(other.msg);
			callback = std::move(other.callback);
			return *this;
		};

		inline WaitingEntry(WaitingEntry&& other)
		{
			canId = std::move(other.canId);
			address = std::move(other.address);
			isRead = std::move(other.isRead);
			sent = std::move(other.sent);
			msg = std::move(other.msg);
			callback = std::move(other.callback);
		};
	};

	static inline std::mutex waitingOnMutex_;
	static std::vector<WaitingEntry> waitingOn_;

	static void
	addWaitingEntry(uint8_t canId, Address address, bool isRead, const modm::can::Message& msg);

	static void
	addWaitingEntry(uint8_t canId, Address address, bool isRead, const modm::can::Message& msg,
					std::function<void(const uint8_t, Value)>&& func);
};

namespace detail
{
inline void
uploadMessage(uint8_t nodeId, Address address, modm::can::Message& message);

inline void
downloadMessage(uint8_t nodeId, Address address, const Value& value, modm::can::Message& message);

};  // namespace detail
}  // namespace modm_canopen
#include "sdo_client_impl.hpp"
#endif