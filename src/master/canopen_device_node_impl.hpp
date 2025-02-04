#ifndef CANOPEN_CANOPEN_DEVICE_NODE_HPP
#error "Do not include this file directly, use canopen_device_node.hpp instead"
#endif

namespace modm_canopen
{

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

	auto handler = getWriteHandler(address);
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
void
CanopenNode<OD, Protocols...>::sync()
{
	std::unique_lock lock(pdoMutex_);
	for (auto& tpdo : transmitPdos_)
	{
		if (tpdo.isActive()) { tpdo.sync(); }
	}
}

template<typename OD, typename... Protocols>
std::optional<ReadHandlerRT>
CanopenNode<OD, Protocols...>::getReadHandler(Address addr)
{
	std::unique_lock lock(handlerMutex_);
	return accessHandlers.lookupReadHandler(addr);
}

template<typename OD, typename... Protocols>
std::optional<WriteHandlerRT>
CanopenNode<OD, Protocols...>::getWriteHandler(Address addr)
{
	std::unique_lock lock(handlerMutex_);
	return accessHandlers.lookupWriteHandler(addr);
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
	return valueFromBytes(entry->dataType, data);
}

template<typename OD, typename... Protocols>
auto
CanopenNode<OD, Protocols...>::write(Address address, std::span<const uint8_t> data,
									 int8_t size) -> SdoErrorCode
{
	auto entry = ObjectDictionary::map.lookup(address);
	if (!entry) { return SdoErrorCode::ObjectDoesNotExist; }
	if (!entry->isWritable()) { return SdoErrorCode::WriteOfReadOnlyObject; }

	const auto objectSize = getDataTypeSize(entry->dataType);
	const bool sizeIsValid =
		(objectSize <= data.size()) && ((size == -1) || (size == int8_t(objectSize)));
	if (!sizeIsValid) { return SdoErrorCode::UnsupportedAccess; }

	auto handler = getWriteHandler(address);
	if (handler)
	{
		const Value value = valueFromBytes(entry->dataType, data);
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
	auto handler = getReadHandler(address);
	if (handler)
	{
		auto ret = callReadHandler(*handler);
		if (!ret) return SdoErrorCode::GeneralError;
		return *ret;
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
CanopenNode<OD, Protocols...>::processMessage(bool isInSyncWindow,
											  const modm::can::Message& message)
{
	std::unique_lock lock(pdoMutex_);
	for (auto& rpdo : receivePdos_)
	{
		if (rpdo.getTransmitMode().isAsync() ||
			(rpdo.getTransmitMode().isOnSync() && isInSyncWindow))
		{
			rpdo.processMessage(message,
								[this](Address address, Value value) { write(address, value); });
		}
	}
}

template<typename OD, typename... Protocols>
template<typename MessageCallback>
void
CanopenNode<OD, Protocols...>::update(bool isInSync, MessageCallback&& cb)
{
	std::unique_lock lock(pdoMutex_);
	for (auto& tpdo : transmitPdos_)
	{
		if (tpdo.isActive())
		{
			auto message =
				tpdo.nextMessage(isInSync, [this](Address address) { return read(address); });
			if (message) { std::forward<MessageCallback>(cb)(*message); }
		}
	}
}

template<typename OD, typename... Protocols>
void
CanopenNode<OD, Protocols...>::updateHandlers(Map map)
{
	std::unique_lock lock(handlerMutex_);
	accessHandlers = map;
}

template<typename OD, typename... Protocols>
void
CanopenNode<OD, Protocols...>::setValueChanged(Address address)
{
	std::unique_lock lock(pdoMutex_);
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
SdoErrorCode
CanopenNode<OD, Protocols...>::setReceivePdoActive(uint8_t index, bool active)
{
	std::unique_lock lock(pdoMutex_);
	SdoErrorCode ret = SdoErrorCode::NoError;
	if (active)
	{
		ret = receivePdos_[index].setActive();
	} else
	{
		receivePdos_[index].setInactive();
	}
	updateRPDOAddrs();
	return ret;
}

template<typename OD, typename... Protocols>
SdoErrorCode
CanopenNode<OD, Protocols...>::setTransmitPdoActive(uint8_t index, bool active)
{
	std::unique_lock lock(pdoMutex_);
	SdoErrorCode ret = SdoErrorCode::NoError;
	if (active)
	{
		ret = transmitPdos_[index].setActive();
	} else
	{
		transmitPdos_[index].setInactive();
	}
	updateTPDOAddrs();
	return ret;
}

template<typename OD, typename... Protocols>
void
CanopenNode<OD, Protocols...>::setReceivePdo(uint8_t index, ReceivePdo_t rpdo)
{
	std::unique_lock lock(pdoMutex_);
	receivePdos_[index] = rpdo;
	updateRPDOAddrs();
}

template<typename OD, typename... Protocols>
void
CanopenNode<OD, Protocols...>::setTransmitPdo(uint8_t index, TransmitPdo_t tpdo)
{
	std::unique_lock lock(pdoMutex_);
	transmitPdos_[index] = tpdo;
	updateTPDOAddrs();
}

template<typename OD, typename... Protocols>
std::vector<modm_canopen::Address>
CanopenNode<OD, Protocols...>::getActiveTPDOAddrs()
{
	std::unique_lock lock(pdoMutex_);
	return tpdoAddrs_;
}

template<typename OD, typename... Protocols>
std::vector<modm_canopen::Address>
CanopenNode<OD, Protocols...>::getActiveRPDOAddrs()
{
	std::unique_lock lock(pdoMutex_);
	return rpdoAddrs_;
}

template<typename OD, typename... Protocols>
void
CanopenNode<OD, Protocols...>::updateTPDOAddrs()
{
	tpdoAddrs_ = std::vector<modm_canopen::Address>();
	for (auto& pdo : transmitPdos_)
	{
		if (pdo.isActive())
		{
			for (size_t i = 0; i < pdo.mappingCount(); i++)
			{
				auto mapping = pdo.mapping(i);
				bool found = false;
				for (auto& addr : tpdoAddrs_)
				{
					if (mapping.address.index == addr.index &&
						mapping.address.subindex == addr.subindex)
					{
						found = true;
					}
				}
				if (!found) tpdoAddrs_.push_back(mapping.address);
			}
		}
	}
}

template<typename OD, typename... Protocols>
void
CanopenNode<OD, Protocols...>::updateRPDOAddrs()
{
	rpdoAddrs_ = std::vector<modm_canopen::Address>();
	for (auto& pdo : receivePdos_)
	{
		if (pdo.isActive())
		{
			for (size_t i = 0; i < pdo.mappingCount(); i++)
			{
				auto mapping = pdo.mapping(i);
				bool found = false;
				for (auto& addr : rpdoAddrs_)
				{
					if (mapping.address.index == addr.index &&
						mapping.address.subindex == addr.subindex)
					{
						found = true;
					}
				}
				if (!found) rpdoAddrs_.push_back(mapping.address);
			}
		}
	}
}

}  // namespace modm_canopen
