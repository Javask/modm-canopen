#ifndef CANOPEN_CANOPEN_DEVICE_HPP
#define CANOPEN_CANOPEN_DEVICE_HPP

#include <array>
#include <span>
#include <optional>
#include "../handler_map.hpp"
#include "../object_dictionary.hpp"
#include "../receive_pdo.hpp"
#include "../receive_pdo_configurator.hpp"
#include "../transmit_pdo_configurator.hpp"
#include "../transmit_pdo.hpp"
#include "sdo_server.hpp"
namespace modm_canopen
{

template<typename OD, typename... Protocols>
class CanopenDevice
{
public:
	static constexpr uint8_t MaxTPDOCount = 8;
	static constexpr uint8_t MaxRPDOCount = 8;

	using ObjectDictionary = OD;
	using ReceivePdo_t = ReceivePdo<OD>;
	using TransmitPdo_t = TransmitPdo<OD>;

	static void
	initialize(uint8_t nodeId);
	static void
	setNodeId(uint8_t id);
	static uint8_t
	nodeId();

	static void
	setValueChanged(Address address);

	/// call on message reception
	template<typename MessageCallback>
	static void
	processMessage(const modm::can::Message& message, MessageCallback&& cb);

	template<typename MessageCallback>
	static void
	update(MessageCallback&& cb);

private:
	friend ReceivePdoConfigurator<CanopenDevice>;
	friend TransmitPdoConfigurator<CanopenDevice>;
	friend SdoServer<CanopenDevice>;

	using Map = HandlerMap<OD>;

	static constexpr auto
	registerHandlers() -> HandlerMap<OD>;
	static constexpr auto
	constructHandlerMap() -> HandlerMap<OD>;

	static constexpr HandlerMap<OD> accessHandlers = constructHandlerMap();

	static inline uint8_t nodeId_{};

	static inline constinit std::array<ReceivePdo_t, MaxRPDOCount> receivePdos_;
	static inline constinit std::array<TransmitPdo_t, MaxTPDOCount> transmitPdos_;

public:
	// TODO: replace return value with std::expected like type, add error code to read handler
	static auto
	read(Address address) -> std::variant<Value, SdoErrorCode>;
	static auto
	write(Address address, Value value) -> SdoErrorCode;
	static auto
	write(Address address, std::span<const uint8_t> data, int8_t size = -1) -> SdoErrorCode;

	static std::optional<Value>
	toValue(Address address, std::span<const uint8_t> data, int8_t size = -1);

	static uint32_t
	rpdoCanId(uint8_t index);
	static uint32_t
	tpdoCanId(uint8_t index);
	static void
	setReceivePdoActive(uint8_t index, bool active);
	static void
	setTransmitPdoActive(uint8_t index, bool active);
	static void
	setReceivePdo(uint8_t index, ReceivePdo_t rpdo);
	static void
	setTransmitPdo(uint8_t index, TransmitPdo_t tpdo);
};

}  // namespace modm_canopen

#include "canopen_device_impl.hpp"

#endif  // CANOPEN_CANOPEN_DEVICE
