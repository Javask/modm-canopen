#ifndef CANOPEN_CANOPEN_MASTER_HPP
#error "Do not include this file directly, include canopen_master.hpp instead!"
#endif
#include "overloaded.hpp"

namespace modm_canopen
{

template<typename... Devices>
void
CanopenMaster<Devices...>::removeDevice(uint8_t id)
{
	std::unique_lock lock(devicesMutex_);
	devices_.erase(id);
}

template<typename... Devices>
template<typename Device>
Device &
CanopenMaster<Devices...>::addDevice(uint8_t id)
{
	std::unique_lock lock(devicesMutex_);
	devices_.emplace(id, std::move(DevicePtr_t<Device>(new Device(id))));
	return *std::get<DevicePtr_t<Device>>(devices_.at(id));
}

template<typename... Devices>
template<typename Device>
Device &
CanopenMaster<Devices...>::addDevice(uint8_t id, Device::Map map)
{
	std::unique_lock lock(devicesMutex_);
	devices_.emplace(id, std::move(DevicePtr_t<Device>(new Device(id, map))));
	return *std::get<DevicePtr_t<Device>>(devices_.at(id));
}

template<typename... Devices>
template<typename Device>
Device &
CanopenMaster<Devices...>::getDevice(uint8_t id)
{
	std::unique_lock lock(devicesMutex_);
	return *std::get<DevicePtr_t<Device>>(devices_.at(id));
}

template<typename... Devices>
template<typename Device>
Device *
CanopenMaster<Devices...>::tryGetDevice(uint8_t id)
{
	std::unique_lock lock(devicesMutex_);
	if (!devices_.contains(id)) return nullptr;
	if (!std::holds_alternative<DevicePtr_t<Device>>(devices_.at(id))) return nullptr;
	return std::get<DevicePtr_t<Device>>(devices_.at(id)).get();
}

template<typename... Devices>
void
CanopenMaster<Devices...>::setValueChangedAll(Address address)
{
	std::unique_lock lock(devicesMutex_);
	for (auto &device : devices_)
	{
		std::visit(overloaded{[](std::monostate) {},
							  [&address](auto &&device) { device->setValueChanged(address); }},
				   device.second);
	}
}

template<typename... Devices>
bool
CanopenMaster<Devices...>::setValueChanged(uint8_t canID, Address address)
{
	std::unique_lock lock(devicesMutex_);
	for (auto &device : devices_)
	{
		auto ret = std::visit(overloaded{[](std::monostate) { return false; },
										 [canID, &address](auto &&device) {
											 if (device->nodeId() == canID)
											 {
												 device->setValueChanged(address);
												 return true;
											 }
											 return false;
										 }},
							  device.second);
		if (ret) return true;
	}
	return false;
}

template<typename... Devices>
auto
CanopenMaster<Devices...>::write(uint8_t id, Address address, Value value) -> SdoErrorCode
{
	std::unique_lock lock(devicesMutex_);
	for (auto &device : devices_)
	{

		auto temp = std::visit(
			overloaded{[](std::monostate) { return std::optional<SdoErrorCode>{}; },
					   [id, address, value](auto &&device) {
						   if (device->nodeId() == id)
						   {
							   return std::optional<SdoErrorCode>{device->write(address, value)};
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
	std::unique_lock lock(devicesMutex_);
	for (auto &device : devices_)
	{
		auto temp =
			std::visit(overloaded{[](std::monostate) { return std::optional<SdoErrorCode>{}; },
								  [id, address, data, size](auto &&device) {
									  if (device->nodeId() == id)
									  {
										  return std::optional<SdoErrorCode>{
											  device->write(address, data, size)};
									  }
									  return std::optional<SdoErrorCode>{};
								  }},
					   device.second);
		if (temp.has_value()) return *temp;
	}
	return SdoErrorCode::GeneralError;
}

template<typename... Devices>
std::optional<Value>
CanopenMaster<Devices...>::toValue(uint8_t id, Address address, std::span<const uint8_t> data,
								   int8_t size)
{
	std::unique_lock lock(devicesMutex_);
	for (auto &device : devices_)
	{

		auto temp = std::visit(overloaded{[](std::monostate) { return std::optional<Value>{}; },
										  [id, address, data, size](auto &&device) {
											  if (device->nodeId() == id)
											  {
												  return device->toValue(address, data, size);
											  }
											  return std::optional<Value>{};
										  }},
							   device.second);
		if (temp.has_value()) return *temp;
	}
	return {};
}

template<typename... Devices>
auto
CanopenMaster<Devices...>::read(uint8_t id, Address address) -> std::variant<Value, SdoErrorCode>
{
	std::unique_lock lock(devicesMutex_);
	for (auto &device : devices_)
	{

		auto temp = std::visit(
			overloaded{
				[](std::monostate) { return std::optional<std::variant<Value, SdoErrorCode>>{}; },
				[id, address](auto &&device) {
					if (device->nodeId() == id)
					{
						return std::optional<std::variant<Value, SdoErrorCode>>{
							device->read(address)};
					}
					return std::optional<std::variant<Value, SdoErrorCode>>{};
				}},
			device.second);
		if (temp.has_value()) return *temp;
	}
	return SdoErrorCode::GeneralError;
}

template<typename... Devices>
template<typename MessageCallback>
void
CanopenMaster<Devices...>::processMessage(const modm::can::Message &message, MessageCallback &&cb)
{
	{
		std::unique_lock lock(devicesMutex_);
		for (auto &pair : devices_)
		{
			std::visit(overloaded{[](std::monostate) {},
								  [&message](auto &&arg) {
									  arg->processMessage(isInSyncWindow(), message);
								  }},
					   pair.second);
		}
	}
	SdoClient_t::processMessage(message, std::forward<MessageCallback>(cb));
}

template<typename... Devices>
template<typename MessageCallback>
void
CanopenMaster<Devices...>::update(MessageCallback &&cb)
{
	{
		std::unique_lock lock(devicesMutex_);
		for (auto &pair : devices_)
		{
			std::visit(overloaded{[](std::monostate) {},
								  [&cb](auto &&arg) {
									  arg->update(isInSyncWindow(),
												  std::forward<MessageCallback>(cb));
								  }},
					   pair.second);
		}
	}

	SdoClient_t::update(cb);

	{
		std::unique_lock lock(syncTimerMutex_);
		if (syncTimer_.execute()) { sendSync(std::forward<MessageCallback>(cb)); }
	}
}

template<typename... Devices>
bool
CanopenMaster<Devices...>::isInSyncWindow()
{
	std::unique_lock lock(syncTimerMutex_);
	if (lastSyncTime_.time_since_epoch().count() == 0) return false;
	const auto now = modm::PreciseClock::now();
	return (now - lastSyncTime_ < syncWindowDuration_);
}

template<typename... Devices>
template<typename MessageCallback>
void
CanopenMaster<Devices...>::sendSync(MessageCallback &&sendMessage)
{
	if (syncCounterOverflow_ != 0)
	{
		lastSyncCounter_++;
		if (lastSyncCounter_ >= syncCounterOverflow_) { lastSyncCounter_ = 0; }
		const modm::can::Message msg(syncCobId_, 1, &lastSyncCounter_, false);
		sendMessage(msg);
	}
	else{
		modm::can::Message msg(syncCobId_, 0);
		msg.setExtended(false);
		sendMessage(msg);
	}
	lastSyncTime_ = modm::PreciseClock::now();
}

template<typename... Devices>
uint32_t
CanopenMaster<Devices...>::tpdoCanId(uint8_t nodeId, uint8_t index)
{
	return (0x100 * (index + 1) + 0x100) + nodeId;  // Reverse than in device
}
template<typename... Devices>
uint32_t
CanopenMaster<Devices...>::rpdoCanId(uint8_t nodeId, uint8_t index)
{
	return (0x100 * (index + 1) + 0x80) + nodeId;  // Reverse than in device
}

template<typename... Devices>
template<typename OD>
void
CanopenMaster<Devices...>::setRPDO(uint8_t sourceId, uint8_t pdoId, ReceivePdo<OD> &pdo)
{
	auto canId = rpdoCanId(sourceId, pdoId);
	pdo.setCanId(canId);

	std::unique_lock lock(devicesMutex_);
	if (devices_.count(sourceId))
	{
		std::visit(overloaded{[](std::monostate) {},
							  [sourceId, pdoId, &pdo](auto &&arg) {
								  using T = std::remove_reference<decltype(*arg)>::type;
								  if constexpr (std::is_same_v<typename T::ObjectDictionary, OD>)
									  arg->setReceivePdo(pdoId, pdo);
							  }},
				   devices_[sourceId]);
	}
}
template<typename... Devices>
template<typename OD>
void
CanopenMaster<Devices...>::setTPDO(uint8_t destinationId, uint8_t pdoId, TransmitPdo<OD> &pdo)
{
	auto canId = tpdoCanId(destinationId, pdoId);
	pdo.setCanId(canId);

	std::unique_lock lock(devicesMutex_);
	if (devices_.contains(destinationId))
	{
		std::visit(overloaded{[](std::monostate) {},
							  [destinationId, pdoId, &pdo](auto &&arg) {
								  using T = std::remove_reference<decltype(*arg)>::type;
								  if constexpr (std::is_same_v<typename T::ObjectDictionary, OD>)
									  arg->setTransmitPdo(pdoId, pdo);
							  }},
				   devices_[destinationId]);
	}
}

template<typename... Devices>
SdoErrorCode
CanopenMaster<Devices...>::setRPDOActive(uint8_t sourceId, uint8_t pdoId, bool active)
{

	std::unique_lock lock(devicesMutex_);
	if (devices_.contains(sourceId))
	{
		return std::visit(overloaded{[](std::monostate) { return SdoErrorCode::PdoMappingError; },
									 [sourceId, pdoId, active](auto &&arg) {
										 return arg->setReceivePdoActive(pdoId, active);
									 }},
						  devices_[sourceId]);
	}
	return SdoErrorCode::PdoMappingError;
}
template<typename... Devices>
SdoErrorCode
CanopenMaster<Devices...>::setTPDOActive(uint8_t destinationId, uint8_t pdoId, bool active)
{

	std::unique_lock lock(devicesMutex_);
	if (devices_.contains(destinationId))
	{
		return std::visit(overloaded{[](std::monostate) { return SdoErrorCode::PdoMappingError; },
									 [destinationId, pdoId, active](auto &&arg) {
										 return arg->setTransmitPdoActive(pdoId, active);
									 }},
						  devices_[destinationId]);
	}
	return SdoErrorCode::PdoMappingError;
}

template<typename... Devices>
template<typename MessageCallback>
void
CanopenMaster<Devices...>::setRemoteRPDOActive(uint8_t remoteId, uint8_t pdoId, bool active,
											   MessageCallback &&sendMessage)
{

	const uint16_t rpdoCommParamAddr = 0x1400 + pdoId;
	const auto rpdoCobIdAddr = Address{rpdoCommParamAddr, 1};
	const uint32_t rpdoCobId =
		tpdoCanId(remoteId, pdoId) |
		(active ? 0 : 0x8000'0000);  // Needs to be tpdoCanId, since they are
									 // reversed on master...
									 // TODO find a way to make that consistent
	SdoClient_t::requestWrite(remoteId, rpdoCobIdAddr, (uint32_t)rpdoCobId,
							  std::forward<MessageCallback>(sendMessage));
}
template<typename... Devices>
template<typename MessageCallback>
void
CanopenMaster<Devices...>::setRemoteTPDOActive(uint8_t remoteId, uint8_t pdoId, bool active,
											   MessageCallback &&sendMessage)
{
	uint16_t tpdoCommParamAddr = 0x1800 + pdoId;
	const auto tpdoCobIdAddr = Address{tpdoCommParamAddr, 1};
	const uint32_t tpdoCobId =
		rpdoCanId(remoteId, pdoId) |
		(active ? 0 : 0x8000'0000);  // Needs to be rpdoCanId, since they are
									 // reversed on master...
									 // TODO find a way to make that consistent
	SdoClient_t::requestWrite(remoteId, tpdoCobIdAddr, (uint32_t)tpdoCobId,
							  std::forward<MessageCallback>(sendMessage));
}

template<typename... Devices>
template<typename OD, typename MessageCallback>
void
CanopenMaster<Devices...>::configureRemoteRPDO(uint8_t remoteId, uint8_t pdoId, TransmitPdo<OD> pdo,
											   MessageCallback &&sendMessage)
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
											   uint16_t inhibitTime_100us,
											   MessageCallback &&sendMessage)
{
	pdo.setInactive();
	setRemoteTPDOActive(remoteId, pdoId, false, std::forward<MessageCallback>(sendMessage));
	uint16_t tpdoCommParamAddr = 0x1800 + pdoId;
	const auto tpdoCobId = Address{tpdoCommParamAddr, 1};
	SdoClient_t::requestWrite(remoteId, tpdoCobId, (uint32_t)pdo.cobId(),
							  std::forward<MessageCallback>(sendMessage));

	const auto tpdoInhibitTime = Address{tpdoCommParamAddr, 3};
	SdoClient_t::requestWrite(remoteId, tpdoInhibitTime, inhibitTime_100us,
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

template<typename... Devices>
std::vector<modm_canopen::Address>
CanopenMaster<Devices...>::getActiveTPDOAddrs(uint8_t id)
{

	std::unique_lock lock(devicesMutex_);
	if (!devices_.contains(id)) return {};
	return std::visit(
		overloaded{[](std::monostate) { return std::vector<modm_canopen::Address>(); },
				   [](auto &&arg) { return arg->getActiveTPDOAddrs(); }},
		devices_[id]);
}

template<typename... Devices>
std::vector<modm_canopen::Address>
CanopenMaster<Devices...>::getActiveRPDOAddrs(uint8_t id)
{

	std::unique_lock lock(devicesMutex_);
	if (!devices_.contains(id)) return {};
	return std::visit(
		overloaded{[](std::monostate) { return std::vector<modm_canopen::Address>(); },
				   [](auto &&arg) { return arg->getActiveRPDOAddrs(); }},
		devices_[id]);
}

}  // namespace modm_canopen
