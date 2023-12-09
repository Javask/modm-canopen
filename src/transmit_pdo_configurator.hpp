#ifndef CANOPEN_TRANSMIT_PDO_CONFIGURATOR_HPP
#define CANOPEN_TRANSMIT_PDO_CONFIGURATOR_HPP

#include <cstdint>
#include "object_dictionary_common.hpp"
#include <modm/debug/logger.hpp>

namespace modm_canopen
{

template<typename Device>
class TransmitPdoConfigurator
{
public:
	template<uint8_t pdo>
	constexpr void
	registerPdoConfigObjects(Device::Map& map)
	{
		auto& tpdo = Device::transmitPdos_[pdo];
		// highest sub-index supported
		map.template setReadHandler<Address{0x1800 + pdo, 0}>(+[]() -> uint8_t { return 5; });

		// RPDO COB-ID
		map.template setReadHandler<Address{0x1800 + pdo, 1}>(+[]() -> uint32_t {
			MODM_LOG_DEBUG << "tpdo " << pdo << " 0x" << modm::hex << tpdo.cobId() << modm::endl;
			return tpdo.cobId();
		});

		map.template setWriteHandler<Address{0x1800 + pdo, 1}>(+[](uint32_t cobId) {
			MODM_LOG_DEBUG << "set tpdo " << pdo << " 0x" << modm::hex << cobId << modm::endl;
			return setTransmitPdoCobId(pdo, cobId);
		});

		map.template setReadHandler<Address{0x1800 + pdo, 2}>(
			// 0xFF: async
			+[]() -> uint8_t { return 0xFF; });

		map.template setWriteHandler<Address{0x1800 + pdo, 2}>(
			// 0xFF: async
			+[](uint8_t transmitMode) -> SdoErrorCode {
				return transmitMode == 0xFF ? SdoErrorCode::NoError
											: SdoErrorCode::UnsupportedAccess;
			});

		map.template setReadHandler<Address{0x1800 + pdo, 3}>(
			+[]() -> uint16_t { return tpdo.inhibitTime(); });

		map.template setWriteHandler<Address{0x1800 + pdo, 3}>(
			+[](uint16_t inhibitTime) { return tpdo.setInhibitTime(inhibitTime); });

		map.template setReadHandler<Address{0x1800 + pdo, 5}>(
			+[]() -> uint16_t { return tpdo.eventTimeout(); });

		map.template setWriteHandler<Address{0x1800 + pdo, 5}>(
			+[](uint16_t timeout_ms) { return tpdo.setEventTimeout(timeout_ms); });
	}

	template<uint8_t pdo, uint8_t mappingIndex>
	constexpr void
	registerMappingObjects(Device::Map& map)
	{
		auto& tpdo = Device::transmitPdos_[pdo];
		map.template setReadHandler<Address{0x1A00 + pdo, mappingIndex + 1}>(
			+[]() -> uint32_t { return tpdo.mapping(mappingIndex).encode(); });

		map.template setWriteHandler<Address{0x1A00 + pdo, mappingIndex + 1}>(
			+[](uint32_t mapping) {
				return tpdo.setMapping(mappingIndex, PdoMapping::decode(mapping));
			});
	}

	template<uint8_t pdo>
	constexpr void
	registerMappingConfigObjects(Device::Map& map)
	{
		auto& tpdos = Device::transmitPdos_;
		// mapping count
		map.template setReadHandler<Address{0x1A00 + pdo, 0}>(
			+[]() -> uint8_t { return tpdos[pdo].mappingCount(); });

		map.template setWriteHandler<Address{0x1A00 + pdo, 0}>(
			+[](uint8_t count) { return tpdos[pdo].setMappingCount(count); });
		registerMappingObjects<pdo, 0>(map);
		registerMappingObjects<pdo, 1>(map);
		registerMappingObjects<pdo, 2>(map);
		registerMappingObjects<pdo, 3>(map);
		registerMappingObjects<pdo, 4>(map);
		registerMappingObjects<pdo, 5>(map);
		registerMappingObjects<pdo, 6>(map);
		registerMappingObjects<pdo, 7>(map);
	}

	template<uint8_t id>
	constexpr void
	registerHandlerRecursive(Device::Map& map)
	{
		registerPdoConfigObjects<id>(map);
		if constexpr (id != 0) { registerHandlerRecursive<id - 1>(map); }
		registerMappingConfigObjects<id>(map);
	}

	constexpr void
	registerHandlers(Device::Map& map)
	{
		if constexpr (Device::MaxTPDOCount != 0)
			registerHandlerRecursive<Device::MaxTPDOCount - 1>(map);
	}

private:
	static SdoErrorCode
	setTransmitPdoCobId(uint_fast8_t index, uint32_t cobId)
	{
		auto& tpdo = Device::transmitPdos_[index];
		return tpdo.setCOBId(cobId);
	}
};

}  // namespace modm_canopen

#endif  // CANOPEN_TRANSMIT_PDO_CONFIGURATOR_HPP
