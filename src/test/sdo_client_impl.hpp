#ifndef CANOPEN_SDO_CLIENT_HPP
#error "Do not include this file directly, include sdo_client.hpp instead!"
#endif
#include <modm/debug/logger.hpp>
namespace modm_canopen
{
using namespace std::literals;
template<typename Device>
std::vector<typename SdoClient<Device>::WaitingEntry> SdoClient<Device>::waitingOn{};

template<typename Device>
template<typename MessageCallback>
void
SdoClient<Device>::update(MessageCallback&& sendMessage)
{
	constexpr modm::Clock::duration timeout{100ms};
	auto now = modm::Clock::now();
	for (auto& wait : waitingOn)
	{
		auto timesince = now - wait.sent;
		if (timesince > timeout)
		{
			sendMessage(wait.msg);
			wait.sent = now;
		}
	}
}

template<typename Device>
template<typename MessageCallback>
void
SdoClient<Device>::processMessage(const modm::can::Message& request,
								  MessageCallback&& responseCallback)
{

	if (request.getLength() != 8) return;
	if (request.isExtended()) return;

	constexpr uint8_t ResponseMask = 0b111'0'00'0'0;
	constexpr uint8_t uploadResponse = 0b010'0'00'0'0;
	constexpr uint8_t downloadResponse = 0b011'0'00'0'0;
	constexpr uint8_t abortResponse = 0b100'0'00'0'0;
	constexpr uint8_t sizeIndicated = 0b000'0'00'0'1;

	const Address address{.index = uint16_t((request.data[2] << 8) | request.data[1]),
						  .subindex = request.data[3]};

	const bool hasSize = (request.data[0] & sizeIndicated) == sizeIndicated;
	int_fast8_t size = -1;
	if (hasSize) { size = (4 - ((request.data[0] & 0b1100) >> 2)); }

	const bool isUpload = (request.data[0] & ResponseMask) == uploadResponse;
	const bool isDownload = (request.data[0] & ResponseMask) == downloadResponse;
	const bool isAbort = (request.data[0] & ResponseMask) == abortResponse;

	for (auto it = waitingOn.begin(); it != waitingOn.end(); ++it)
	{
		if (it->canId + (uint32_t)0x580 == request.getIdentifier() && address == it->address)
		{

			if (isUpload)
			{
				waitingOn.erase(it);
				const SdoErrorCode error =
					Device::write(address, std::span<const uint8_t>{&request.data[4], 4}, size);
				std::forward<MessageCallback>(responseCallback)(address, error);
				return;
			}
			if (isDownload)
			{
				waitingOn.erase(it);
				std::forward<MessageCallback>(responseCallback)(address, SdoErrorCode::NoError);
				return;
			}
			if (isAbort)
			{
				waitingOn.erase(it);
				static_assert(sizeof(SdoErrorCode) == 4);
				SdoErrorCode err = *((SdoErrorCode*)&request.data[4]);
				std::forward<MessageCallback>(responseCallback)(address, err);
				return;
			}
		}
	}
}

template<typename Device>
template<typename MessageCallback>
void
SdoClient<Device>::requestRead(uint8_t canId, Address address, MessageCallback&& sendMessage)
{
	auto msg = detail::uploadMessage(canId, address);
	addWaitingEntry(canId, address, true, msg);
	sendMessage(msg);
}

template<typename Device>
template<typename MessageCallback>
void
SdoClient<Device>::requestWrite(uint8_t canId, Address address, MessageCallback&& sendMessage)
{
	auto value = Device::read(address);
	if (std::get_if<Value>(value))
	{
		auto msg = detail::downloadMessage(canId, address, std::get<Value>(value));
		addWaitingEntry(canId, address, false, msg);
		sendMessage(msg);
	}
}

template<typename Device>
template<typename MessageCallback>
void
SdoClient<Device>::requestWrite(uint8_t canId, Address address, const Value& value,
								MessageCallback&& sendMessage)
{
	auto msg = detail::downloadMessage(canId, address, value);
	addWaitingEntry(canId, address, false, msg);
	sendMessage(msg);
}

template<typename Device>
void
SdoClient<Device>::addWaitingEntry(uint8_t canId, Address address, bool isRead,
								   modm::can::Message msg)
{
	WaitingEntry entry{.canId = canId,
					   .address = address,
					   .isRead = isRead,
					   .sent = modm::Clock::now(),
					   .msg = msg};
	waitingOn.push_back(entry);
}

template<typename Device>
bool
SdoClient<Device>::waiting()
{
	return !waitingOn.empty();
}

auto
detail::uploadMessage(uint8_t nodeId, Address address) -> modm::can::Message
{
	modm::can::Message message{uint32_t(0x600 + nodeId), 8};
	message.setExtended(false);
	message.data[0] = 0b010'00000;
	message.data[1] = address.index & 0xFF;
	message.data[2] = (address.index & 0xFF'00) >> 8;
	message.data[3] = address.subindex;
	return message;
}

auto
detail::downloadMessage(uint8_t nodeId, Address address, const Value& value) -> modm::can::Message
{
	modm::can::Message message{uint32_t(0x600 + nodeId), 8};
	message.setExtended(false);
	const auto sizeFlags = (0b11 & (4 - getValueSize(value))) << 2;
	message.data[0] = 0b001'0'00'1'1 | sizeFlags;
	message.data[1] = address.index & 0xFF;
	message.data[2] = (address.index & 0xFF'00) >> 8;
	message.data[3] = address.subindex;
	valueToBytes(value, &message.data[4]);
	return message;
}

}  // namespace modm_canopen