#ifndef CANOPEN_CANOPEN_MASTER_HPP
#define CANOPEN_CANOPEN_MASTER_HPP

#include <array>
#include <map>
#include <variant>
#include <tuple>
#include <optional>
#include <memory>
#include <span>
#include <mutex>

#include <modm/processing/timer.hpp>

#include "../object_dictionary.hpp"
#include "../receive_pdo.hpp"
#include "../receive_pdo_configurator.hpp"
#include "../transmit_pdo_configurator.hpp"
#include "../transmit_pdo.hpp"
#include "sdo_client.hpp"
#include "canopen_device_node.hpp"

using namespace std::literals;

namespace modm_canopen
{

template<typename... Devices>
class CanopenMaster
{
public:
	using SdoClient_t = SdoClient<CanopenMaster<Devices...>>;
	template<typename Device>
	using DevicePtr_t = std::unique_ptr<Device>;
	using Device_t = std::variant<std::monostate, DevicePtr_t<Devices>...>;

	static void
	setValueChangedAll(Address address);

	static bool
	setValueChanged(uint8_t canID, Address address);

	/// call on message reception
	template<typename MessageCallback>
	static void
	processMessage(const modm::can::Message& message, MessageCallback&& responseCallback);

	template<typename MessageCallback>
	static void
	update(MessageCallback&& cb);

	template<typename Device>
	static Device&
	addDevice(uint8_t id);

	template<typename Device>
	static Device&
	addDevice(uint8_t id, Device::Map map);

	template<typename Device>
	static Device&
	getDevice(uint8_t id);

	template<typename Device>
	static Device*
	tryGetDevice(uint8_t id);

	static void
	removeDevice(uint8_t id);

	static std::vector<modm_canopen::Address>
	getActiveTPDOAddrs(uint8_t id);

	static std::vector<modm_canopen::Address>
	getActiveRPDOAddrs(uint8_t id);

	static bool
	isInSyncWindow();
	static uint8_t
	syncCounter();

private:
	friend SdoClient_t;
	static inline uint8_t masterId_{0};

	static inline std::mutex devicesMutex_{};
	static inline std::map<uint8_t, Device_t> devices_{};

	static inline std::mutex syncTimerMutex_{};
	static inline uint8_t syncCounterOverflow_{0};
	static inline uint8_t lastSyncCounter_{0};
	static inline uint32_t syncCobId_{0x80};
	static inline modm::PrecisePeriodicTimer syncTimer_{50ms};
	static inline modm::PreciseClock::duration syncWindowDuration_{25ms};
	static inline modm::PreciseClock::time_point lastSyncTime_{};

	template<typename MessageCallback>
	static void
	sendSync(MessageCallback&& sendMessage);

public:
	// TODO: replace return value with std::expected like type, add error code to read handler
	static auto
	read(uint8_t id, Address address) -> std::variant<Value, SdoErrorCode>;
	static auto
	write(uint8_t id, Address address, Value value) -> SdoErrorCode;
	static auto
	write(uint8_t id, Address address, std::span<const uint8_t> data,
		  int8_t size = -1) -> SdoErrorCode;

	static std::optional<Value>
	toValue(uint8_t id, Address address, std::span<const uint8_t> data, int8_t size = -1);

	static uint32_t
	rpdoCanId(uint8_t nodeId, uint8_t index);
	static uint32_t
	tpdoCanId(uint8_t nodeId, uint8_t index);

	template<typename OD>
	static void
	setRPDO(uint8_t sourceId, uint8_t pdoId, ReceivePdo<OD>& pdo);
	template<typename OD>
	static void
	setTPDO(uint8_t destinationId, uint8_t pdoId, TransmitPdo<OD>& pdo);

	static SdoErrorCode
	setRPDOActive(uint8_t sourceId, uint8_t pdoId, bool active);
	static SdoErrorCode
	setTPDOActive(uint8_t destinationId, uint8_t pdoId, bool active);

	template<typename MessageCallback>
	static void
	setRemoteRPDOActive(uint8_t remoteId, uint8_t pdoId, bool active,
						MessageCallback&& sendMessage);

	template<typename MessageCallback>
	static void
	setRemoteTPDOActive(uint8_t remoteId, uint8_t pdoId, bool active,
						MessageCallback&& sendMessage);

	template<typename OD, typename MessageCallback>
	static void
	configureRemoteRPDO(uint8_t remoteId, uint8_t pdoId, TransmitPdo<OD> pdo,
						MessageCallback&& sendMessage);

	template<typename OD, typename MessageCallback>
	static void
	configureRemoteTPDO(uint8_t remoteId, uint8_t pdoId, ReceivePdo<OD> pdo,
						uint16_t inhibitTime_100us, MessageCallback&& sendMessage);
};

}  // namespace modm_canopen

#include "canopen_master_impl.hpp"

#endif