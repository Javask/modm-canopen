#ifndef CANOPEN_CIA402_PROTOCOL_HPP
#error "Do not include this file directly, use cia402_protocol.hpp instead"
#endif

namespace modm_canopen
{
template<typename Device, typename MessageCallback>
void
CiA402::update(MessageCallback &&)
{}

template<typename Device, typename MessageCallback>
void
CiA402::processMessage(const modm::can::Message &, MessageCallback &&)
{}

template<typename ObjectDictionary>
constexpr void
CiA402::registerHandlers(modm_canopen::HandlerMap<ObjectDictionary> &)
{}
}  // namespace modm_canopen