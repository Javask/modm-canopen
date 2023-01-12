#ifndef CANOPEN_CANOPEN_MASTER_HPP
#error "Do not include this file directly, include canopen_master.hpp instead!"
#endif

namespace modm_canopen
{
namespace detail
{

template<Address address>
struct missing_read_handler
{
	static_assert(address == Address{}, "Read handler not registered for at least one object");
};

template<Address address>
struct missing_write_handler
{
	static_assert(address == Address{}, "Write handler not registered for at least one object");
};

}  // namespace detail

template<typename OD, typename... Protocols>
auto
CanopenMaster<OD, Protocols...>::write(Address address, Value value) -> SdoErrorCode
{
	auto entry = OD::map.lookup(address);
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
auto
CanopenMaster<OD, Protocols...>::write(Address address, std::span<const uint8_t> data, int8_t size)
	-> SdoErrorCode
{
	auto entry = OD::map.lookup(address);
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
CanopenMaster<OD, Protocols...>::read(Address address) -> std::variant<Value, SdoErrorCode>
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
template<typename MessageCallback>
void
CanopenMaster<OD, Protocols...>::processMessage(const modm::can::Message& message,
												MessageCallback&& cb)
{
	for (auto& rpdo : receivePdos_)
	{
		rpdo.processMessage(message, [](Address address, Value value) { write(address, value); });
	}
	SdoClient_t::processMessage(message, std::forward<MessageCallback>(cb));
}

template<typename OD, typename... Protocols>
template<typename MessageCallback>
void
CanopenMaster<OD, Protocols...>::update(MessageCallback&& cb)
{
	for (auto& tpdo : transmitPdos_)
	{
		if (tpdo.isActive())
		{
			auto message = tpdo.nextMessage([](Address address) { return read(address); });
			if (message) { std::forward<MessageCallback>(cb)(*message); }
		}
	}
}

template<typename OD, typename... Protocols>
void
CanopenMaster<OD, Protocols...>::setValueChanged(Address address)
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
constexpr auto
CanopenMaster<OD, Protocols...>::registerHandlers() -> HandlerMap<OD>
{
	HandlerMap<OD> handlers;
	(Protocols{}.registerHandlers(handlers), ...);

	return handlers;
}

template<typename OD, typename... Protocols>
constexpr auto
CanopenMaster<OD, Protocols...>::constructHandlerMap() -> HandlerMap<OD>
{
	constexpr HandlerMap<OD> handlers = registerHandlers();
	detail::missing_read_handler<findMissingReadHandler(handlers)>();
	detail::missing_write_handler<findMissingWriteHandler(handlers)>();
	return handlers;
}

template<typename OD, typename... Protocols>
uint32_t
CanopenMaster<OD, Protocols...>::tpdoCanId(uint8_t nodeId, uint8_t index)
{
	return (0x100 * (index + 1) + 0x100) | nodeId;  // Reverse than in device
}
template<typename OD, typename... Protocols>
uint32_t
CanopenMaster<OD, Protocols...>::rpdoCanId(uint8_t nodeId, uint8_t index)
{
	return (0x100 * (index + 1) + 0x80) | nodeId;  // Reverse than in device
}

template<typename OD, typename... Protocols>
void
CanopenMaster<OD, Protocols...>::setRPDO(uint8_t sourceId, uint8_t pdoId, ReceivePdo_t& pdo)
{
	auto canId = rpdoCanId(sourceId, pdoId);
	pdo.setCanId(canId);
	for (auto& rpdo : receivePdos_)
	{
		if (rpdo.canId() == canId)
		{
			rpdo = pdo;
			return;
		}
	}
	receivePdos_.push_back(pdo);
}
template<typename OD, typename... Protocols>
void
CanopenMaster<OD, Protocols...>::setTPDO(uint8_t destinationId, uint8_t pdoId, TransmitPdo_t& pdo)
{
	auto canId = tpdoCanId(destinationId, pdoId);
	pdo.setCanId(canId);
	for (auto& tpdo : transmitPdos_)
	{
		if (tpdo.canId() == canId)
		{
			tpdo = pdo;
			return;
		}
	}
	transmitPdos_.push_back(pdo);
}

template<typename OD, typename... Protocols>
SdoErrorCode
CanopenMaster<OD, Protocols...>::setRPDOActive(uint8_t sourceId, uint8_t pdoId, bool active)
{
	auto canId = rpdoCanId(sourceId, pdoId);
	for (auto& rpdo : receivePdos_)
	{
		if (rpdo.canId() == canId)
		{
			if (active)
				return rpdo.setActive();
			else
				return rpdo.setInactive();
		}
	}
	return SdoErrorCode::PdoMappingError;
}
template<typename OD, typename... Protocols>
SdoErrorCode
CanopenMaster<OD, Protocols...>::setTPDOActive(uint8_t destinationId, uint8_t pdoId, bool active)
{
	auto canId = tpdoCanId(destinationId, pdoId);
	for (auto& tpdo : transmitPdos_)
	{
		if (tpdo.canId() == canId)
		{
			if (active)
				return tpdo.setActive();
			else
				return tpdo.setInactive();
		}
	}
	return SdoErrorCode::PdoMappingError;
}

template<typename OD, typename... Protocols>
template<typename MessageCallback>
void
CanopenMaster<OD, Protocols...>::setRemoteRPDOActive(uint8_t remoteId, uint8_t pdoId, bool active,
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
template<typename OD, typename... Protocols>
template<typename MessageCallback>
void
CanopenMaster<OD, Protocols...>::setRemoteTPDOActive(uint8_t remoteId, uint8_t pdoId, bool active,
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

template<typename OD, typename... Protocols>
template<typename MessageCallback>
void
CanopenMaster<OD, Protocols...>::configureRemoteRPDO(uint8_t remoteId, uint8_t pdoId,
													 TransmitPdo_t pdo,
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
template<typename OD, typename... Protocols>
template<typename MessageCallback>
void
CanopenMaster<OD, Protocols...>::configureRemoteTPDO(uint8_t remoteId, uint8_t pdoId,
													 ReceivePdo_t pdo,
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
