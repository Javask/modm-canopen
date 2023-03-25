#ifndef CANOPEN_CANOPEN_MASTER_HPP
#error "Do not include this file directly, include canopen_master.hpp instead!"
#endif

namespace modm_canopen
{

template<class... Ts>
struct overloaded : Ts...
{
	using Ts::operator()...;
};  // i hate this

template<typename... Devices>
void
CanopenMaster<Devices...>::removeDevice(uint8_t id)
{
	devices_.erase(id);
}

template<typename... Devices>
template<typename Device>
Device&
CanopenMaster<Devices...>::addDevice(uint8_t id)
{
	devices_.emplace(id, Device(id));
	return std::get<Device>(devices_.at(id));
}

template<typename... Devices>
template<typename Device>
Device&
CanopenMaster<Devices...>::getDevice(uint8_t id)
{
	return std::get<Device>(devices_.at(id));
}

template<typename... Devices>
void
CanopenMaster<Devices...>::setValueChanged(Address address)
{
	for (auto& device : devices_)
	{
		std::visit(overloaded{[](std::monostate) {},
							  [&address](auto&& device) { device.setValueChanged(address); }},
				   device.second);
	}
}

template<typename... Devices>
auto
CanopenMaster<Devices...>::write(uint8_t id, Address address, Value value) -> SdoErrorCode
{
	for (auto& device : devices_)
	{

		auto temp = std::visit(
			overloaded{[](std::monostate) { return std::optional<SdoErrorCode>{}; },
					   [id, address, value](auto&& device) {
						   if (device.nodeId() == id)
						   {
							   return std::optional<SdoErrorCode>{device.write(address, value)};
						   }
						   return std::optional<SdoErrorCode>{};
					   }},
			device.second);
		if (temp.has_value()) return *temp;
	}
	return SdoErrorCode::GeneralError;
}

template<typename... Devices>
auto
CanopenMaster<Devices...>::write(uint8_t id, Address address, std::span<const uint8_t> data,
								 int8_t size) -> SdoErrorCode
{
	for (auto& device : devices_)
	{

		auto temp =
			std::visit(overloaded{[](std::monostate) { return std::optional<SdoErrorCode>{}; },
								  [id, address, data, size](auto&& device) {
									  if (device.nodeId() == id)
									  {
										  return std::optional<SdoErrorCode>{
											  device.write(address, data, size)};
									  }
									  return std::optional<SdoErrorCode>{};
								  }},
					   device.second);
		if (temp.has_value()) return *temp;
	}
	return SdoErrorCode::GeneralError;
}

template<typename... Devices>
auto
CanopenMaster<Devices...>::read(uint8_t id, Address address) -> std::variant<Value, SdoErrorCode>
{
	for (auto& device : devices_)
	{

		auto temp = std::visit(
			overloaded{
				[](std::monostate) { return std::optional<std::variant<Value, SdoErrorCode>>{}; },
				[id, address](auto&& device) {
					if (device.nodeId() == id)
					{
						return std::optional<std::variant<Value, SdoErrorCode>>{
							device.read(address)};
					}
					return std::optional<std::variant<Value, SdoErrorCode>>{};
				}},
			device);
		if (temp.has_value()) return *temp;
	}
	return SdoErrorCode::GeneralError;
}

template<typename... Devices>
template<typename MessageCallback>
void
CanopenMaster<Devices...>::processMessage(const modm::can::Message& message, MessageCallback&& cb)
{
	for (auto& pair : devices_)
	{
		std::visit(overloaded{[](std::monostate) {},
							  [&message](auto&& arg) { arg.processMessage(message); }},
				   pair.second);
	}
	SdoClient_t::processMessage(message, std::forward<MessageCallback>(cb));
}

template<typename... Devices>
template<typename MessageCallback>
void
CanopenMaster<Devices...>::update(MessageCallback&& cb)
{
	for (auto& pair : devices_)
	{
		std::visit(overloaded{[](std::monostate) {},
							  [&cb](auto&& arg) { arg.update(std::forward<MessageCallback>(cb)); }},
				   pair.second);
	}
	SdoClient_t::update(cb);
}

template<typename... Devices>
uint32_t
CanopenMaster<Devices...>::tpdoCanId(uint8_t nodeId, uint8_t index)
{
	return (0x100 * (index + 1) + 0x100) | nodeId;  // Reverse than in device
}
template<typename... Devices>
uint32_t
CanopenMaster<Devices...>::rpdoCanId(uint8_t nodeId, uint8_t index)
{
	return (0x100 * (index + 1) + 0x80) | nodeId;  // Reverse than in device
}

template<typename... Devices>
template<typename OD>
void
CanopenMaster<Devices...>::setRPDO(uint8_t sourceId, uint8_t pdoId, ReceivePdo<OD>& pdo)
{
	auto canId = rpdoCanId(sourceId, pdoId);
	pdo.setCanId(canId);
	if (devices_.count(sourceId))
	{
		std::visit(
			overloaded{[](std::monostate) {},
					   [sourceId, pdoId, &pdo](auto&& arg) { arg.setReceivePdo(pdoId, pdo); }},
			devices_[sourceId]);
	}
}
template<typename... Devices>
template<typename OD>
void
CanopenMaster<Devices...>::setTPDO(uint8_t destinationId, uint8_t pdoId, TransmitPdo<OD>& pdo)
{
	auto canId = tpdoCanId(destinationId, pdoId);
	pdo.setCanId(canId);
	if (devices_.contains(destinationId))
	{
		std::visit(
			overloaded{[](std::monostate) {}, [destinationId, pdoId, &pdo](
												  auto&& arg) { arg.setTransmitPdo(pdoId, pdo); }},
			devices_[destinationId]);
	}
}

template<typename... Devices>
SdoErrorCode
CanopenMaster<Devices...>::setRPDOActive(uint8_t sourceId, uint8_t pdoId, bool active)
{
	if (devices_.contains(sourceId))
	{
		return std::visit(overloaded{[](std::monostate) { return SdoErrorCode::PdoMappingError; },
									 [sourceId, pdoId, active](auto&& arg) {
										 return arg.setRPDOActive(sourceId, pdoId, active);
									 }},
						  devices_[sourceId]);
	}
	return SdoErrorCode::PdoMappingError;
}
template<typename... Devices>
SdoErrorCode
CanopenMaster<Devices...>::setTPDOActive(uint8_t destinationId, uint8_t pdoId, bool active)
{
	if (devices_.contains(destinationId))
	{
		return std::visit(overloaded{[](std::monostate) { return SdoErrorCode::PdoMappingError; },
									 [destinationId, pdoId, active](auto&& arg) {
										 return arg.setTPDOActive(destinationId, pdoId, active);
									 }},
						  devices_[destinationId]);
	}
	return SdoErrorCode::PdoMappingError;
}

template<typename... Devices>
template<typename MessageCallback>
void
CanopenMaster<Devices...>::setRemoteRPDOActive(uint8_t remoteId, uint8_t pdoId, bool active,
											   MessageCallback&& sendMessage)
{

	const uint16_t rpdoCommParamAddr = 0x1400 + pdoId;
	const auto rpdoCobIdAddr = Address{rpdoCommParamAddr, 1};
	const uint32_t rpdoCobId =
		tpdoCanId(remoteId, pdoId) |
		(active ? 0 : 0x8000'0000);  // Needs to be tpdoCanId, since they are reversed on master...
									 // TODO find a way to make that consistent
	SdoClient_t::requestWrite(remoteId, rpdoCobIdAddr, (uint32_t)rpdoCobId,
							  std::forward<MessageCallback>(sendMessage));
}
template<typename... Devices>
template<typename MessageCallback>
void
CanopenMaster<Devices...>::setRemoteTPDOActive(uint8_t remoteId, uint8_t pdoId, bool active,
											   MessageCallback&& sendMessage)
{
	uint16_t tpdoCommParamAddr = 0x1800 + pdoId;
	const auto tpdoCobIdAddr = Address{tpdoCommParamAddr, 1};
	const uint32_t tpdoCobId =
		rpdoCanId(remoteId, pdoId) |
		(active ? 0 : 0x8000'0000);  // Needs to be rpdoCanId, since they are reversed on master...
									 // TODO find a way to make that consistent
	SdoClient_t::requestWrite(remoteId, tpdoCobIdAddr, (uint32_t)tpdoCobId,
							  std::forward<MessageCallback>(sendMessage));
}

template<typename... Devices>
template<typename OD, typename MessageCallback>
void
CanopenMaster<Devices...>::configureRemoteRPDO(uint8_t remoteId, uint8_t pdoId, TransmitPdo<OD> pdo,
											   MessageCallback&& sendMessage)
{
	pdo.setInactive();
	setRemoteRPDOActive(remoteId, pdoId, false, std::forward<MessageCallback>(sendMessage));
	const uint16_t rpdoCommParamAddr = 0x1400 + pdoId;
	const auto rpdoCobId = Address{rpdoCommParamAddr, 1};
	SdoClient_t::requestWrite(remoteId, rpdoCobId, (uint32_t)pdo.cobId(),
							  std::forward<MessageCallback>(sendMessage));

	const uint16_t rpdoMapParamAddr = 0x1600 + pdoId;

	for (uint8_t i = 0; i < pdo.mappingCount(); i++)
	{
		const auto rpdoMappingAddr = Address{rpdoMapParamAddr, (uint8_t)(i + 1)};
		SdoClient_t::requestWrite(remoteId, rpdoMappingAddr, (uint32_t)pdo.mapping(i).encode(),
								  std::forward<MessageCallback>(sendMessage));
	}

	const auto rpdoMappingCount = Address{rpdoMapParamAddr, 0};
	SdoClient_t::requestWrite(remoteId, rpdoMappingCount, (uint8_t)pdo.mappingCount(),
							  std::forward<MessageCallback>(sendMessage));
	setRemoteRPDOActive(remoteId, pdoId, true, std::forward<MessageCallback>(sendMessage));
}
template<typename... Devices>
template<typename OD, typename MessageCallback>
void
CanopenMaster<Devices...>::configureRemoteTPDO(uint8_t remoteId, uint8_t pdoId, ReceivePdo<OD> pdo,
											   MessageCallback&& sendMessage)
{
	pdo.setInactive();
	setRemoteTPDOActive(remoteId, pdoId, false, std::forward<MessageCallback>(sendMessage));
	uint16_t tpdoCommParamAddr = 0x1800 + pdoId;
	const auto tpdoCobId = Address{tpdoCommParamAddr, 1};
	SdoClient_t::requestWrite(remoteId, tpdoCobId, (uint32_t)pdo.cobId(),
							  std::forward<MessageCallback>(sendMessage));

	uint16_t tpdoMapParamAddr = 0x1A00 + pdoId;

	for (uint8_t i = 0; i < pdo.mappingCount(); i++)
	{
		const auto tpdoMappingAddr = Address{tpdoMapParamAddr, (uint8_t)(i + 1)};
		SdoClient_t::requestWrite(remoteId, tpdoMappingAddr, (uint32_t)pdo.mapping(i).encode(),
								  std::forward<MessageCallback>(sendMessage));
	}
	const auto tpdoMappingCount = Address{tpdoMapParamAddr, 0};
	SdoClient_t::requestWrite(remoteId, tpdoMappingCount, (uint8_t)pdo.mappingCount(),
							  std::forward<MessageCallback>(sendMessage));

	setRemoteTPDOActive(remoteId, pdoId, true, std::forward<MessageCallback>(sendMessage));
}

}  // namespace modm_canopen
