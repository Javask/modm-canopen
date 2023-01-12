#ifndef CANOPEN_CANOPEN_MASTER_HPP
#define CANOPEN_CANOPEN_MASTER_HPP

#include <array>
#include <vector>
#include <span>
#include "object_dictionary.hpp"
#include "handler_map.hpp"
#include "receive_pdo.hpp"
#include "receive_pdo_configurator.hpp"
#include "transmit_pdo_configurator.hpp"
#include "transmit_pdo.hpp"
#include "sdo_client.hpp"

namespace modm_canopen
{

template<typename OD, typename... Protocols>
class CanopenMaster
{
public:
	using ObjectDictionary = OD;
	using ReceivePdo_t = ReceivePdo<OD>;
	using TransmitPdo_t = TransmitPdo<OD>;
	using SdoClient_t = SdoClient<CanopenMaster>;

	static void
	setValueChanged(Address address);

	/// call on message reception
	template<typename MessageCallback>
	static void
	processMessage(const modm::can::Message& message, MessageCallback&& responseCallback);

	template<typename MessageCallback>
	static void
	update(MessageCallback&& cb);

private:
	friend SdoClient_t;

	using Map = HandlerMap<OD>;

	static constexpr auto
	registerHandlers() -> HandlerMap<OD>;
	static constexpr auto
	constructHandlerMap() -> HandlerMap<OD>;

	static constexpr HandlerMap<OD> accessHandlers = constructHandlerMap();

	static inline uint8_t nodeId_{};

	static inline std::vector<ReceivePdo_t> receivePdos_{};
	static inline std::vector<TransmitPdo_t> transmitPdos_{};

public:
	// TODO: replace return value with std::expected like type, add error code to read handler
	static auto
	read(Address address) -> std::variant<Value, SdoErrorCode>;
	static auto
	write(Address address, Value value) -> SdoErrorCode;
	static auto
	write(Address address, std::span<const uint8_t> data, int8_t size = -1) -> SdoErrorCode;

	static uint32_t
	rpdoCanId(uint8_t nodeId, uint8_t index);
	static uint32_t
	tpdoCanId(uint8_t nodeId, uint8_t index);

	static void
	setRPDO(uint8_t sourceId, uint8_t pdoId, ReceivePdo_t& pdo);
	static void
	setTPDO(uint8_t destinationId, uint8_t pdoId, TransmitPdo_t& pdo);

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

	template<typename MessageCallback>
	static void
	configureRemoteRPDO(uint8_t remoteId, uint8_t pdoId, TransmitPdo_t pdo,
						MessageCallback&& sendMessage);

	template<typename MessageCallback>
	static void
	configureRemoteTPDO(uint8_t remoteId, uint8_t pdoId, ReceivePdo_t pdo,
						MessageCallback&& sendMessage);
};

}  // namespace modm_canopen

#include "canopen_master_impl.hpp"

#endif