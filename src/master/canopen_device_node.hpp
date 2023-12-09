#ifndef CANOPEN_CANOPEN_MASTER_NODE_HPP
#define CANOPEN_CANOPEN_DEVICE_NODE_HPP

#include <array>
#include <span>
#include <atomic>
#include "../object_dictionary.hpp"
#include "handler_map_rt.hpp"
#include "../receive_pdo.hpp"
#include "../receive_pdo_configurator.hpp"
#include "../transmit_pdo_configurator.hpp"
#include "../transmit_pdo.hpp"
#include "inverse_object_dictionary.hpp"
namespace modm_canopen
{

template<typename OD, typename... Protocols>
class CanopenNode
{
private:
	uint8_t nodeId_{};

public:
	static constexpr uint8_t MaxTPDOCount = 8;
	static constexpr uint8_t MaxRPDOCount = 8;

	using ObjectDictionary = inverse<OD>;  // Invert Read/Write to make sense in the master
	using ReceivePdo_t = ReceivePdo<ObjectDictionary>;
	using TransmitPdo_t = TransmitPdo<ObjectDictionary>;

	CanopenNode(uint8_t nodeId) : nodeId_(nodeId), accessHandlers(constructHandlerMap(nodeId)){};

	template<typename SdoClient, typename MessageCallback>
	void
	initialize(MessageCallback&& sendMessage);

	void
	setValueChanged(Address address);

	void
	processMessage(const modm::can::Message& message);

	template<typename MessageCallback>
	void
	update(MessageCallback&& cb);

private:
	using Map = HandlerMapRT<ObjectDictionary>;

	auto
	registerHandlers(uint8_t id) -> Map;
	auto
	constructHandlerMap(uint8_t id) -> Map;

	Map accessHandlers;
	std::atomic_uint configuredPDOCount_;
	std::array<ReceivePdo_t, MaxRPDOCount> receivePdos_;
	std::array<TransmitPdo_t, MaxTPDOCount> transmitPdos_;

public:
	bool
	initialized();

	inline void
	setNodeId(uint8_t id)
	{
		nodeId_ = id;
	}

	inline uint8_t
	nodeId() const
	{
		return nodeId_;
	}

	// TODO: replace return value with std::expected like type, add error code to read handler
	auto
	read(Address address) -> std::variant<Value, SdoErrorCode>;
	auto
	write(Address address, Value value) -> SdoErrorCode;
	auto
	write(Address address, std::span<const uint8_t> data, int8_t size = -1) -> SdoErrorCode;

	std::optional<Value>
	toValue(Address address, std::span<const uint8_t> data, int8_t size = -1);

	void
	setReceivePdoActive(uint8_t index, bool active);
	void
	setTransmitPdoActive(uint8_t index, bool active);
	void
	setReceivePdo(uint8_t index, ReceivePdo_t rpdo);
	void
	setTransmitPdo(uint8_t index, TransmitPdo_t tpdo);

	template<typename SdoClient, typename MessageCallback>
	void
	setRemoteRPDOActive(uint8_t pdoId, bool active, MessageCallback&& sendMessage);

	template<typename SdoClient, typename MessageCallback>
	void
	setRemoteTPDOActive(uint8_t pdoId, bool active, MessageCallback&& sendMessage);

	template<typename SdoClient, typename MessageCallback>
	void
	configureRemoteRPDO(uint8_t pdoId, TransmitPdo_t pdo, MessageCallback&& sendMessage);

	template<typename SdoClient, typename MessageCallback>
	void
	configureRemoteTPDO(uint8_t pdoId, ReceivePdo_t pdo, uint16_t inhibitTime_100us,
						MessageCallback&& sendMessage);

private:
	static uint32_t
	rpdoCanId(uint8_t nodeId, uint8_t index);
	static uint32_t
	tpdoCanId(uint8_t nodeId, uint8_t index);
};

}  // namespace modm_canopen

#include "canopen_device_node_impl.hpp"

#endif  // CANOPEN_CANOPEN_DEVICE_NODE_HPP
