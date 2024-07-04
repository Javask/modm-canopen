#ifndef CANOPEN_CANOPEN_MASTER_NODE_HPP
#define CANOPEN_CANOPEN_DEVICE_NODE_HPP

#include <array>
#include <span>
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
	static constexpr uint8_t MaxTPDOCount = 4;
	static constexpr uint8_t MaxRPDOCount = 4;

	using ObjectDictionary = inverse<OD>;  // Invert Read/Write to make sense in the master
	using Map = HandlerMapRT<ObjectDictionary>;
	using ReceivePdo_t = ReceivePdo<ObjectDictionary>;
	using TransmitPdo_t = TransmitPdo<ObjectDictionary>;

	CanopenNode(uint8_t nodeId, Map map) : nodeId_(nodeId), accessHandlers(map){};

	CanopenNode(uint8_t nodeId) : CanopenNode(nodeId, constructHandlerMap(nodeId)){};

	void
	updateHandlers(Map map);

	void
	setValueChanged(Address address);

	void
	processMessage(const modm::can::Message& message);

	template<typename MessageCallback>
	void
	update(MessageCallback&& cb);

private:
	auto
	registerHandlers(uint8_t id) -> Map;
	auto
	constructHandlerMap(uint8_t id) -> Map;

	Map accessHandlers;

	std::array<ReceivePdo_t, MaxRPDOCount> receivePdos_;
	std::array<TransmitPdo_t, MaxTPDOCount> transmitPdos_;

public:
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
};

}  // namespace modm_canopen

#include "canopen_device_node_impl.hpp"

#endif  // CANOPEN_CANOPEN_DEVICE_NODE_HPP
