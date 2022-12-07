#ifndef CANOPEN_CANOPEN_DEVICE_HPP
#define CANOPEN_CANOPEN_DEVICE_HPP

#include <array>
#include <span>
#include "object_dictionary.hpp"
#include "handler_map.hpp"
#include "receive_pdo.hpp"
#include "receive_pdo_configurator.hpp"
#include "transmit_pdo_configurator.hpp"
#include "transmit_pdo.hpp"
#include "sdo_server.hpp"


namespace modm_canopen
{

template<typename OD, typename... Protocols>
class CanopenDevice
{
public:
    using ObjectDictionary = OD;

    static void initialize(uint8_t nodeId) { setNodeId(nodeId); }

    static void setNodeId(uint8_t id);
    static uint8_t nodeId();

    static void setValueChanged(Address address);

    /// call on message reception
    template<typename MessageCallback>
    static void processMessage(const modm::can::Message& message, MessageCallback&& cb);

    template<typename MessageCallback>
    static void update(MessageCallback&& cb);

private:
    friend ReceivePdoConfigurator<CanopenDevice>;
    friend TransmitPdoConfigurator<CanopenDevice>;
    friend SdoServer<CanopenDevice>;

    using Map = HandlerMap<OD>;

    // TODO: replace return value with std::expected like type, add error code to read handler
    static auto read(Address address) -> std::variant<Value, SdoErrorCode>;
    static auto write(Address address, Value value) -> SdoErrorCode;
    static auto write(Address address, std::span<const uint8_t> data, int8_t size = -1) -> SdoErrorCode;

    static constexpr auto registerHandlers() -> HandlerMap<OD>;
    static constexpr auto constructHandlerMap() -> HandlerMap<OD>;

    static constexpr HandlerMap<OD> accessHandlers = constructHandlerMap();

    static inline constinit SdoServer<CanopenDevice> sdoServer_;
    static inline uint8_t nodeId_{};

    static inline constinit std::array<ReceivePdo<OD>, 4> receivePdos_;
    static inline constinit std::array<TransmitPdo<OD>, 4> transmitPdos_;

public:
    using ReceivePdo_t = ReceivePdo<OD>;
    using TransmitPdo_t = TransmitPdo<OD>;

    static inline void setReceivePdoActive(uint8_t index, bool active){
        receivePdos_[index].setActive(active);
    }
    static inline void setTransmitPdoActive(uint8_t index, bool active){
        transmitPdos_[index].setActive(active);
    }
    static inline void setReceivePdo(uint8_t index, ReceivePdo_t rpdo){
        receivePdos_[index] = rpdo;
    }
    static inline void setTransmitPdo(uint8_t index, TransmitPdo_t tpdo){
        transmitPdos_[index] = tpdo;
    }
};

}

#include "canopen_device_impl.hpp"

#endif // CANOPEN_CANOPEN_DEVICE
