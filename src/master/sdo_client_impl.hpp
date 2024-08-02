#ifndef CANOPEN_SDO_CLIENT_HPP
#error "Do not include this file directly, include sdo_client.hpp instead!"
#endif
#include <modm/debug/logger.hpp>
namespace modm_canopen
{
using namespace std::literals;
template<typename Device>
std::vector<typename SdoClient<Device>::WaitingEntry> SdoClient<Device>::waitingOn_{};

template<typename Device>
template<typename MessageCallback>
void
SdoClient<Device>::update(MessageCallback&& sendMessage)
{
	constexpr modm::Clock::duration timeout{100ms};
	auto now = modm::Clock::now();
	for (auto& wait : waitingOn_)
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

	std::unique_lock lock(waitingOnMutex_);
	for (auto it = waitingOn_.begin(); it != waitingOn_.end(); ++it)
	{

		if (it->canId + (uint32_t)0x580 == request.getIdentifier() && address == it->address)
		{

			if (isUpload)
			{
				SdoErrorCode error = SdoErrorCode::NoError;
				if (it->callback)
				{
					auto val = Device::toValue(it->canId, address,
											   std::span<const uint8_t>{&request.data[4], 4}, size);
					if (val) { it->callback(it->canId, *val); }
				} else
				{
					error = Device::write(it->canId, address,
										  std::span<const uint8_t>{&request.data[4], 4}, size);
				}
				std::forward<MessageCallback>(responseCallback)(it->canId, address, error);
				waitingOn_.erase(it);
				return;
			}
			if (isDownload)
			{
				std::forward<MessageCallback>(responseCallback)(it->canId, address,
																SdoErrorCode::NoError);
				waitingOn_.erase(it);
				return;
			}
			if (isAbort)
			{
				static_assert(sizeof(SdoErrorCode) == 4);
				SdoErrorCode err = *((SdoErrorCode*)&request.data[4]);
				std::forward<MessageCallback>(responseCallback)(it->canId, address, err);
				waitingOn_.erase(it);
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
	modm::can::Message msg;
	detail::uploadMessage(canId, address, msg);
	addWaitingEntry(canId, address, true, msg);
	sendMessage(msg);
}

template<typename Device>
template<typename MessageCallback>
void
SdoClient<Device>::requestRead(uint8_t canId, Address address,
							   std::function<void(const uint8_t, Value)>&& valueCallback,
							   MessageCallback&& sendMessage)
{
	modm::can::Message msg;
	detail::uploadMessage(canId, address, msg);
	addWaitingEntry(canId, address, true, msg, std::move(valueCallback));
	sendMessage(msg);
}

template<typename Device>
template<typename MessageCallback>
bool
SdoClient<Device>::requestWrite(uint8_t canId, Address address, MessageCallback&& sendMessage)
{
	auto value = Device::read(canId, address);
	if (std::holds_alternative<Value>(value))
	{
		modm::can::Message msg;
		detail::downloadMessage(canId, address, std::get<Value>(value), msg);
		addWaitingEntry(canId, address, false, msg);
		sendMessage(msg);
		return true;
	}
	return false;
}

template<typename Device>
template<typename MessageCallback>
void
SdoClient<Device>::requestWrite(uint8_t canId, Address address, const Value& value,
								MessageCallback&& sendMessage)
{
	modm::can::Message msg;
	detail::downloadMessage(canId, address, value, msg);
	addWaitingEntry(canId, address, false, msg);
	sendMessage(msg);
}

template<typename Device>
void
SdoClient<Device>::addWaitingEntry(uint8_t canId, Address address, bool isRead,
								   const modm::can::Message& msg)
{
	WaitingEntry entry{};
	entry.canId = canId;
	entry.address = address;
	entry.isRead = isRead;
	entry.sent = modm::Clock::now();
	entry.msg = msg;
	entry.callback = {};
	waitingOn_.push_back(entry);
}

template<typename Device>
void
SdoClient<Device>::addWaitingEntry(uint8_t canId, Address address, bool isRead,
								   const modm::can::Message& msg,
								   std::function<void(const uint8_t, Value)>&& func)
{
	WaitingEntry entry{};
	entry.canId = canId;
	entry.address = address;
	entry.isRead = isRead;
	entry.sent = modm::Clock::now();
	entry.msg = msg;
	entry.callback = std::move(func);
	waitingOn_.push_back(entry);
}

template<typename Device>
bool
SdoClient<Device>::waiting()
{
	std::unique_lock lock(waitingOnMutex_);
	return !waitingOn_.empty();
}

template<typename Device>
bool
SdoClient<Device>::waitingOn(uint8_t id)
{
	std::unique_lock lock(waitingOnMutex_);
	for (auto& entry : waitingOn_)
	{
		if (entry.canId == id) return true;
	}
	return false;
}

void
detail::uploadMessage(uint8_t nodeId, Address address, modm::can::Message& message)
{
	message = modm::can::Message{uint32_t(0x600 + nodeId), 8};
	message.setExtended(false);
	message.data[0] = 0b010'00000;
	message.data[1] = address.index & 0xFF;
	message.data[2] = (address.index & 0xFF'00) >> 8;
	message.data[3] = address.subindex;
}

void
detail::downloadMessage(uint8_t nodeId, Address address, const Value& value,
						modm::can::Message& message)
{
	message = modm::can::Message{uint32_t(0x600 + nodeId), 8};
	message.setExtended(false);
	const auto sizeFlags = (0b11 & (4 - getValueSize(value))) << 2;
	message.data[0] = 0b001'0'00'1'1 | sizeFlags;
	message.data[1] = address.index & 0xFF;
	message.data[2] = (address.index & 0xFF'00) >> 8;
	message.data[3] = address.subindex;
	valueToBytes(value, &message.data[4]);
}

}  // namespace modm_canopen