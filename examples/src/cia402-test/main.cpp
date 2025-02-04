#include <modm-canopen/device/canopen_device.hpp>
#include <modm-canopen/cia402/cia402_protocol.hpp>
#include <modm-canopen/generated/test_od.hpp>
#include <modm/platform/can/socketcan.hpp>

#include <iostream>
#include <thread>
#include <modm/processing/timer.hpp>
#include <modm/debug/logger.hpp>

uint32_t value2002 = 42;

using modm_canopen::Address;
using modm_canopen::CanopenDevice;
using modm_canopen::SdoErrorCode;
using modm_canopen::generated::test_OD;

struct Test
{
	template<typename Device, typename MessageCallback>
	static void
	update(MessageCallback&&)
	{}

	template<typename Device, typename MessageCallback>
	static void
	processMessage(const modm::can::Message&, MessageCallback&&)
	{}

	template<typename ObjectDictionary>
	constexpr void
	registerHandlers(modm_canopen::HandlerMap<ObjectDictionary>& map)
	{
		map.template setReadHandler<Address{0x2001, 0}>(+[]() { return uint8_t(10); });

		map.template setReadHandler<Address{0x2002, 0}>(+[]() { return value2002; });

		map.template setWriteHandler<Address{0x2002, 0}>(+[](uint32_t value) {
			MODM_LOG_INFO << "setting 0x2002,0 to " << value << modm::endl;
			value2002 = value;
			return SdoErrorCode::NoError;
		});
	}
};

int
main()
{
	using Device = CanopenDevice<test_OD, Test, modm_canopen::cia402::CiA402<0>>;
	const uint8_t nodeId = 5;

	Device::initialize(nodeId, modm_canopen::Identity{.deviceType_ = 402,
													  .vendorId_ = 0xdeadbeef,
													  .productCode_ = 0,
													  .revisionId_ = 1,
													  .serialNumber_ = 1});

	modm::platform::SocketCan can;
	const bool success = can.open("vcan0");
	if (!success) { MODM_LOG_ERROR << "Opening device vcan0 failed" << modm::endl; }

	auto sendMessage = [&can](const modm::can::Message& message) { can.sendMessage(message); };

	/*
	// manually setup default TPDO1 mapping
	// TODO: proper public APi
	Device::transmitPdos_[0].setInactive();
	Device::transmitPdos_[0].setMapping(0, PdoMapping{Address{0x2002, 0}, 32});
	Device::transmitPdos_[0].setMappingCount(1);
	Device::transmitPdos_[0].setActive();
	Device::transmitPdos_[0].setEventTimeout(500);
	*/

	// call setValueChanged() when a TPDO mappable value changed
	// to trigger asynchronous PDO transmissions
	Device::setValueChanged(Address{0x2002, 0});

	modm::PeriodicTimer timer{100s};
	modm::PreciseClock::time_point lastSync_{};

	while (true)
	{
		if (timer.execute())
		{
			// modm_canopen::cia402::CiA402<0>::setError();
		}
		if (can.isMessageAvailable())
		{
			modm::can::Message message{};
			can.getMessage(message);
			if (message.isExtended())
			{
				message.identifier &= 0x1FFFFFFF;
			} else
			{
				message.identifier &= 0x7FF;
			}
			if (message.getIdentifier() == 0x80)
			{
				const auto now = modm::PreciseClock::now();
				if (lastSync_.time_since_epoch().count() != 0)
				{
					MODM_LOG_DEBUG << "Received Sync after " << (now - lastSync_).count() << "us"
								   << modm::endl;
				}
				lastSync_ = now;
			}
			Device::processMessage(message, sendMessage);
		}
		Device::update(sendMessage);
	}
}
