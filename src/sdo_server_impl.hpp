#ifndef CANOPEN_SDO_SERVER_HPP
#error "Do not include this file directly, include sdo_server.hpp instead!"
#endif

namespace modm_canopen
{

template<typename Device>
template<typename C>
void
SdoServer<Device>::processMessage(const modm::can::Message& request, C&& cb)
{
	constexpr uint8_t commandUpload = 0b010'0'00'0'0;
	constexpr uint8_t commandUploadMask = 0b111'0'00'0'0;
	constexpr uint8_t commandExpediteDownload = 0b001'0'00'1'0;
	constexpr uint8_t commandDownloadMask = 0b111'0'00'1'0;
	constexpr uint8_t sizeIndicated = 0b000'0'00'0'1;

	if (request.identifier == rxCOBId() && request.getLength() == 8)
	{
		const Address address{.index = uint16_t((request.data[2] << 8) | request.data[1]),
							  .subindex = request.data[3]};
		// TODO: clean-up, refactor into functions
		if ((request.data[0] & commandUploadMask) == commandUpload)
		{
			auto result = Device::read(address);
			if (const SdoErrorCode* error = std::get_if<SdoErrorCode>(&result); error)
			{
				modm::can::Message msg;
				detail::transferAbort(txCOBId(), address, *error, msg);
				std::forward<C>(cb)(msg);
			} else
			{
				// std::get_if can't return nullptr
				// there are only two possible types: SdoErrorCode and Value
				const Value& value = *std::get_if<Value>(&result);
				if (valueSupportsExpediteTransfer(value))
				{
					modm::can::Message msg;
					detail::uploadResponse(txCOBId(), address, value, msg);
					std::forward<C>(cb)(msg);
				} else
				{
					modm::can::Message msg;
					detail::transferAbort(txCOBId(), address, SdoErrorCode::UnsupportedAccess, msg);
					std::forward<C>(cb)(msg);
				}
			}
		} else if ((request.data[0] & commandDownloadMask) == commandExpediteDownload)
		{
			const uint8_t type = request.data[0];
			const int_fast8_t size = (type & sizeIndicated) ? (4 - ((type & 0b1100) >> 2)) : -1;
			const SdoErrorCode error =
				Device::write(address, std::span<const uint8_t>{&request.data[4], 4}, size);
			if (error == SdoErrorCode::NoError)
			{
				modm::can::Message msg;
				detail::downloadResponse(txCOBId(), address, msg);
				std::forward<C>(cb)(msg);
			} else
			{
				modm::can::Message msg;
				detail::transferAbort(txCOBId(), address, error, msg);
				std::forward<C>(cb)(msg);
			}
		} else
		{
			modm::can::Message msg;
			detail::transferAbort(txCOBId(), address, SdoErrorCode::UnsupportedAccess, msg);
			std::forward<C>(cb)(msg);
		}
	}
}

template<typename Device>
uint8_t
SdoServer<Device>::nodeId()
{
	return nodeId_;
}

template<typename Device>
void
SdoServer<Device>::setNodeId(uint8_t id)
{
	nodeId_ = id;
}

template<typename Device>
uint32_t
SdoServer<Device>::rxCOBId()
{
	return (uint32_t)nodeId_ + 0x600;
}

template<typename Device>
uint32_t
SdoServer<Device>::txCOBId()
{
	return (uint32_t)nodeId_ + 0x580;
}

void
detail::uploadResponse(uint32_t txCOBId, Address address, const Value& value,
					   modm::can::Message& message)
{
	message = modm::can::Message{txCOBId, 8};
	message.setExtended(false);
	const auto sizeFlags = (0b11 & (4 - getValueSize(value))) << 2;
	message.data[0] = 0b010'0'00'1'1 | sizeFlags;
	message.data[1] = address.index & 0xFF;
	message.data[2] = (address.index & 0xFF'00) >> 8;
	message.data[3] = address.subindex;
	valueToBytes(value, &message.data[4]);
}

void
detail::downloadResponse(uint32_t txCOBId, Address address, modm::can::Message& message)

{
	message = modm::can::Message{txCOBId, 8};
	message.setExtended(false);
	message.data[0] = 0b011'00000;
	message.data[1] = address.index & 0xFF;
	message.data[2] = (address.index & 0xFF'00) >> 8;
	message.data[3] = address.subindex;
}

void
detail::transferAbort(uint32_t txCOBId, Address address, SdoErrorCode error,
					  modm::can::Message& message)
{
	message = modm::can::Message{txCOBId, 8};
	message.setExtended(false);
	message.data[0] = 0b100'00000;
	message.data[1] = address.index & 0xFF;
	message.data[2] = (address.index & 0xFF'00) >> 8;
	message.data[3] = address.subindex;
	static_assert(sizeof(SdoErrorCode) == 4);
	memcpy(&message.data[4], &error, sizeof(SdoErrorCode));
}

}  // namespace modm_canopen
