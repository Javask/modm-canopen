#ifndef CANOPEN_CANOPEN_DEVICE_NODE_HPP
#error "Do not include this file directly, use canopen_device_node.hpp instead"
#endif

namespace modm_canopen
{

template<typename OD, typename... Protocols>
template<typename SdoClient, typename MessageCallback>
void
CanopenNode<OD, Protocols...>::initialize(MessageCallback&& sendMessage)
{
	const uint16_t rpdoCommParamAddrBase = 0x1400;
	const uint16_t tpdoCommParamAddrBase = 0x1800;
	for (uint16_t i = 0; i < MaxRPDOCount; i++)
	{
		SdoClient::requestRead(
			nodeId_, Address{(uint16_t)(rpdoCommParamAddrBase + i), 1},
			[this, i](const uint8_t, Value value) {
				receivePdos_[i].setCanId(std::get<uint32_t>(value));
				MODM_LOG_INFO << "Got COB Id for R" << nodeId_ << ":" << i << ": 0x" << modm::hex
							  << receivePdos_[i].cobId() << modm::endl;
				configuredPDOCount_++;
			},
			std::forward<MessageCallback>(sendMessage));
	}
	for (uint16_t i = 0; i < MaxTPDOCount; i++)
	{
		SdoClient::requestRead(
			nodeId_, Address{(uint16_t)(tpdoCommParamAddrBase + i), 1},
			[this, i](const uint8_t, Value value) {
				transmitPdos_[i].setCanId(std::get<uint32_t>(value));
				MODM_LOG_INFO << "Got COB Id for T" << nodeId_ << ":" << i << ": 0x" << modm::hex
							  << transmitPdos_[i].cobId() << modm::endl;
				configuredPDOCount_++;
			},
			std::forward<MessageCallback>(sendMessage));
	}
}

template<typename OD, typename... Protocols>
bool
CanopenNode<OD, Protocols...>::initialized()
{
	return configuredPDOCount_ == MaxRPDOCount + MaxTPDOCount;
}

template<typename OD, typename... Protocols>
auto
CanopenNode<OD, Protocols...>::write(Address address, Value value) -> SdoErrorCode
{
	auto entry = ObjectDictionary::map.lookup(address);
	if (!entry) { return SdoErrorCode::ObjectDoesNotExist; }
	if (value.index() != static_cast<uint32_t>(entry->dataType))
	{
		return SdoErrorCode::UnsupportedAccess;
	}

	auto handler = accessHandlers.lookupWriteHandler(address);
	if (handler)
	{
		const auto result = callWriteHandler(*handler, value);
		if (result == SdoErrorCode::NoError) { setValueChanged(address); }
		return result;
	} else if (!entry->isWritable())
	{
		return SdoErrorCode::WriteOfReadOnlyObject;
	} else
	{  // TODO: can this happen?
		return SdoErrorCode::UnsupportedAccess;
	}
}

template<typename OD, typename... Protocols>
std::optional<Value>
CanopenNode<OD, Protocols...>::toValue(Address address, std::span<const uint8_t> data, int8_t size)
{
	auto entry = ObjectDictionary::map.lookup(address);
	if (!entry) { return {}; }
	if (!entry->isWritable()) { return {}; }
	const auto objectSize = getDataTypeSize(entry->dataType);
	const bool sizeIsValid =
		(objectSize <= data.size()) && ((size == -1) || (size == int8_t(objectSize)));
	if (!sizeIsValid) { return {}; }
	return valueFromBytes(entry->dataType, data.data());
}

template<typename OD, typename... Protocols>
auto
CanopenNode<OD, Protocols...>::write(Address address, std::span<const uint8_t> data, int8_t size)
	-> SdoErrorCode
{
	auto entry = ObjectDictionary::map.lookup(address);
	if (!entry) { return SdoErrorCode::ObjectDoesNotExist; }
	if (!entry->isWritable()) { return SdoErrorCode::WriteOfReadOnlyObject; }

	const auto objectSize = getDataTypeSize(entry->dataType);
	const bool sizeIsValid =
		(objectSize <= data.size()) && ((size == -1) || (size == int8_t(objectSize)));
	if (!sizeIsValid) { return SdoErrorCode::UnsupportedAccess; }

	auto handler = accessHandlers.lookupWriteHandler(address);
	if (handler)
	{
		const Value value = valueFromBytes(entry->dataType, data.data());
		const auto result = callWriteHandler(*handler, value);
		if (result == SdoErrorCode::NoError) { setValueChanged(address); }

		return result;
	}
	return SdoErrorCode::UnsupportedAccess;
}

template<typename OD, typename... Protocols>
auto
CanopenNode<OD, Protocols...>::read(Address address) -> std::variant<Value, SdoErrorCode>
{
	auto handler = accessHandlers.lookupReadHandler(address);
	if (handler)
	{
		return callReadHandler(*handler);
	} else
	{
		auto entry = OD::map.lookup(address);
		if (!entry)
		{
			return SdoErrorCode::ObjectDoesNotExist;
		} else if (entry->isReadable())
		{
			return SdoErrorCode::ReadOfWriteOnlyObject;
		}
	}
	return SdoErrorCode::UnsupportedAccess;
}

template<typename OD, typename... Protocols>
void
CanopenNode<OD, Protocols...>::processMessage(const modm::can::Message& message)
{
	for (auto& rpdo : receivePdos_)
	{
		rpdo.processMessage(message,
							[this](Address address, Value value) { write(address, value); });
	}
}

template<typename OD, typename... Protocols>
template<typename MessageCallback>
void
CanopenNode<OD, Protocols...>::update(MessageCallback&& cb)
{
	for (auto& tpdo : transmitPdos_)
	{
		if (tpdo.isActive())
		{
			auto message = tpdo.nextMessage([this](Address address) { return read(address); });
			if (message) { std::forward<MessageCallback>(cb)(*message); }
		}
	}
}

template<typename OD, typename... Protocols>
void
CanopenNode<OD, Protocols...>::setValueChanged(Address address)
{
	for (auto& tpdo : transmitPdos_)
	{
		if (tpdo.isActive())
		{
			for (uint_fast8_t i = 0; i < tpdo.mappingCount(); ++i)
			{
				if (tpdo.mapping(i).address == address)
				{
					tpdo.setValueUpdated();
					break;
				}
			}
		}
	}
}

template<typename OD, typename... Protocols>
auto
CanopenNode<OD, Protocols...>::registerHandlers(uint8_t id) -> CanopenNode<OD, Protocols...>::Map
{
	Map handlers;
	(Protocols{}.registerHandlers(id, handlers), ...);

	return handlers;
}

template<typename OD, typename... Protocols>
auto
CanopenNode<OD, Protocols...>::constructHandlerMap(uint8_t id) -> CanopenNode<OD, Protocols...>::Map
{
	Map handlers = registerHandlers(id);
	return handlers;
}

template<typename OD, typename... Protocols>
void
CanopenNode<OD, Protocols...>::setReceivePdoActive(uint8_t index, bool active)
{
	receivePdos_[index].setActive(active);
}

template<typename OD, typename... Protocols>
void
CanopenNode<OD, Protocols...>::setTransmitPdoActive(uint8_t index, bool active)
{
	transmitPdos_[index].setActive(active);
}

template<typename OD, typename... Protocols>
void
CanopenNode<OD, Protocols...>::setReceivePdo(uint8_t index, ReceivePdo_t rpdo)
{
	receivePdos_[index] = rpdo;
}

template<typename OD, typename... Protocols>
void
CanopenNode<OD, Protocols...>::setTransmitPdo(uint8_t index, TransmitPdo_t tpdo)
{
	transmitPdos_[index] = tpdo;
}

template<typename OD, typename... Protocols>
template<typename SdoClient, typename MessageCallback>
void
CanopenNode<OD, Protocols...>::setRemoteRPDOActive(uint8_t pdoId, bool active,
												   MessageCallback&& sendMessage)
{
	const uint16_t rpdoCommParamAddr = 0x1400 + pdoId;
	const auto rpdoCobIdAddr = Address{rpdoCommParamAddr, 1};
	const uint32_t rpdoCobId = receivePdos_[pdoId].cobId() | (active ? 0 : 0x8000'0000);
	SdoClient::requestWrite(nodeId_, rpdoCobIdAddr, (uint32_t)rpdoCobId,
							std::forward<MessageCallback>(sendMessage));
}

template<typename OD, typename... Protocols>
template<typename SdoClient, typename MessageCallback>
void
CanopenNode<OD, Protocols...>::setRemoteTPDOActive(uint8_t pdoId, bool active,
												   MessageCallback&& sendMessage)
{
	uint16_t tpdoCommParamAddr = 0x1800 + pdoId;
	const auto tpdoCobIdAddr = Address{tpdoCommParamAddr, 1};
	const uint32_t tpdoCobId = transmitPdos_[pdoId].cobId() | (active ? 0 : 0x8000'0000);
	SdoClient::requestWrite(nodeId_, tpdoCobIdAddr, (uint32_t)tpdoCobId,
							std::forward<MessageCallback>(sendMessage));
}

template<typename OD, typename... Protocols>
template<typename SdoClient, typename MessageCallback>
void
CanopenNode<OD, Protocols...>::configureRemoteRPDO(uint8_t pdoId, TransmitPdo_t pdo,
												   MessageCallback&& sendMessage)
{
	pdo.setInactive();
	setRemoteRPDOActive<SdoClient, MessageCallback>(pdoId, false,
													std::forward<MessageCallback>(sendMessage));
	const uint16_t rpdoCommParamAddr = 0x1400 + pdoId;
	const auto rpdoCobId = Address{rpdoCommParamAddr, 1};
	SdoClient::requestWrite(nodeId_, rpdoCobId, (uint32_t)pdo.cobId(),
							std::forward<MessageCallback>(sendMessage));

	const uint16_t rpdoMapParamAddr = 0x1600 + pdoId;

	for (uint8_t i = 0; i < pdo.mappingCount(); i++)
	{
		const auto rpdoMappingAddr = Address{rpdoMapParamAddr, (uint8_t)(i + 1)};
		SdoClient::requestWrite(nodeId_, rpdoMappingAddr, (uint32_t)pdo.mapping(i).encode(),
								std::forward<MessageCallback>(sendMessage));
	}

	const auto rpdoMappingCount = Address{rpdoMapParamAddr, 0};
	SdoClient::requestWrite(nodeId_, rpdoMappingCount, (uint8_t)pdo.mappingCount(),
							std::forward<MessageCallback>(sendMessage));
	setRemoteRPDOActive<SdoClient, MessageCallback>(pdoId, true,
													std::forward<MessageCallback>(sendMessage));
}

template<typename OD, typename... Protocols>
template<typename SdoClient, typename MessageCallback>
void
CanopenNode<OD, Protocols...>::configureRemoteTPDO(uint8_t pdoId, ReceivePdo_t pdo,
												   uint16_t inhibitTime_100us,
												   MessageCallback&& sendMessage)
{
	pdo.setInactive();
	setRemoteTPDOActive<SdoClient, MessageCallback>(pdoId, false,
													std::forward<MessageCallback>(sendMessage));
	uint16_t tpdoCommParamAddr = 0x1800 + pdoId;
	const auto tpdoCobId = Address{tpdoCommParamAddr, 1};
	SdoClient::requestWrite(nodeId_, tpdoCobId, (uint32_t)pdo.cobId(),
							std::forward<MessageCallback>(sendMessage));

	const auto tpdoInhibitTime = Address{tpdoCommParamAddr, 3};
	SdoClient::requestWrite(nodeId_, tpdoInhibitTime, inhibitTime_100us,
							std::forward<MessageCallback>(sendMessage));

	uint16_t tpdoMapParamAddr = 0x1A00 + pdoId;

	for (uint8_t i = 0; i < pdo.mappingCount(); i++)
	{
		const auto tpdoMappingAddr = Address{tpdoMapParamAddr, (uint8_t)(i + 1)};
		SdoClient::requestWrite(nodeId_, tpdoMappingAddr, (uint32_t)pdo.mapping(i).encode(),
								std::forward<MessageCallback>(sendMessage));
	}
	const auto tpdoMappingCount = Address{tpdoMapParamAddr, 0};
	SdoClient::requestWrite(nodeId_, tpdoMappingCount, (uint8_t)pdo.mappingCount(),
							std::forward<MessageCallback>(sendMessage));

	setRemoteTPDOActive<SdoClient, MessageCallback>(pdoId, true,
													std::forward<MessageCallback>(sendMessage));
}

template<typename OD, typename... Protocols>
uint32_t
CanopenNode<OD, Protocols...>::tpdoCanId(uint8_t nodeId, uint8_t index)
{
	return (0x100 * (index + 1) + 0x100) + nodeId;  // Reverse than in device
}

template<typename OD, typename... Protocols>
uint32_t
CanopenNode<OD, Protocols...>::rpdoCanId(uint8_t nodeId, uint8_t index)
{
	return (0x100 * (index + 1) + 0x80) + nodeId;  // Reverse than in device
}

}  // namespace modm_canopen
