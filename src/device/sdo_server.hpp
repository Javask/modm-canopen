#ifndef CANOPEN_SDO_SERVER_HPP
#define CANOPEN_SDO_SERVER_HPP

#include <modm/architecture/interface/can_message.hpp>

namespace modm_canopen
{

template<typename Device>
class SdoServer
{
public:
	using ObjectDictionary = Device::ObjectDictionary;

	static uint8_t
	nodeId();
	static void
	setNodeId(uint8_t id);

	static uint32_t
	rxCOBId();
	static uint32_t
	txCOBId();

	template<typename MessageCallback>
	static void
	processMessage(const modm::can::Message& request, MessageCallback&& responseCallback);

	constexpr void
	registerHandlers(Device::Map& map)
	{
		map.template setReadHandler<Address{0x1200, 0}>(+[]() -> uint8_t { return 2; });
		map.template setReadHandler<Address{0x1200, 1}>(+[]() -> uint32_t { return rxCOBId(); });
		map.template setReadHandler<Address{0x1200, 2}>(+[]() -> uint32_t { return txCOBId(); });
	}

private:
	static inline uint8_t nodeId_{};
};

namespace detail
{
inline void
uploadResponse(uint32_t txCOBId, Address address, const Value& value, modm::can::Message& msg);

inline void
downloadResponse(uint32_t txCOBId, Address address, modm::can::Message& msg);

inline void
transferAbort(uint32_t txCOBId, Address address, SdoErrorCode error, modm::can::Message& msg);

};  // namespace detail

}  // namespace modm_canopen

#include "sdo_server_impl.hpp"

#endif  // CANOPEN_CANOPEN_DEVICE
