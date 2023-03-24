#ifndef CANOPEN_SDO_CLIENT_HPP
#define CANOPEN_SDO_CLIENT_HPP
#include <modm/architecture/interface/can_message.hpp>
#include "object_dictionary.hpp"
#include <future>
#include <vector>
#include <cstdint>
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

private:
	struct WaitingEntry
	{
		uint8_t canId;
		Address address;
		bool isRead;
		modm::Clock::time_point sent;
		modm::can::Message msg;
	};

	static std::vector<WaitingEntry> waitingOn;

	static void
	addWaitingEntry(uint8_t canId, Address address, bool isRead, modm::can::Message msg);
};

namespace detail
{
inline auto
uploadMessage(uint8_t nodeId, Address address) -> modm::can::Message;

inline auto
downloadMessage(uint8_t nodeId, Address address, const Value& value) -> modm::can::Message;

};  // namespace detail
}  // namespace modm_canopen
#include "sdo_client_impl.hpp"
#endif