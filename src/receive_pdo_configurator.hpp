#ifndef CANOPEN_RECEIVE_PDO_CONFIGURATOR_HPP
#define CANOPEN_RECEIVE_PDO_CONFIGURATOR_HPP

#include <cstdint>
#include "object_dictionary_common.hpp"

namespace modm_canopen
{

template<typename Device>
class ReceivePdoConfigurator
{
public:
	template<uint8_t pdo>
	constexpr void
	registerPdoConfigObjects(Device::Map& map)
	{
		auto& rpdo = Device::receivePdos_[pdo];
		// highest sub-index supported
		map.template setReadHandler<Address{0x1400 + pdo, 0}>(+[]() -> uint8_t { return 2; });

		// RPDO COB-ID
		map.template setReadHandler<Address{0x1400 + pdo, 1}>(
			+[]() -> uint32_t { return rpdo.cobId(); });

		map.template setWriteHandler<Address{0x1400 + pdo, 1}>(
			+[](uint32_t cobId) { return setReceivePdoCobId(pdo, cobId); });

		// Transmission type, only async supported for now
		map.template setReadHandler<Address{0x1400 + pdo, 2}>(
			// 0xFF: async
			+[]() -> uint8_t { return 0xFF; });
		map.template setWriteHandler<Address{0x1400 + pdo, 2}>(
			// 0xFF: async
			+[](uint8_t transmitMode) -> SdoErrorCode {
				return transmitMode == 0xFF ? SdoErrorCode::NoError
											: SdoErrorCode::UnsupportedAccess;
			});
	}

	template<uint8_t pdo, uint8_t mappingIndex>
	constexpr void
	registerMappingObjects(Device::Map& map)
	{
		auto& rpdo = Device::receivePdos_[pdo];
		map.template setReadHandler<Address{0x1600 + pdo, mappingIndex + 1}>(
			+[]() -> uint32_t { return rpdo.mapping(mappingIndex).encode(); });

		map.template setWriteHandler<Address{0x1600 + pdo, mappingIndex + 1}>(
			+[](uint32_t mapping) {
				return rpdo.setMapping(mappingIndex, PdoMapping::decode(mapping));
			});
	}

	template<uint8_t pdo>
	constexpr void
	registerMappingConfigObjects(Device::Map& map)
	{
		auto& rpdos = Device::receivePdos_;
		// mapping count
		map.template setReadHandler<Address{0x1600 + pdo, 0}>(
			+[]() -> uint8_t { return rpdos[pdo].mappingCount(); });

		map.template setWriteHandler<Address{0x1600 + pdo, 0}>(
			+[](uint8_t count) { return rpdos[pdo].setMappingCount(count); });
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
		if constexpr (Device::MaxRPDOCount != 0)
			registerHandlerRecursive<Device::MaxRPDOCount - 1>(map);
	}

private:
	static SdoErrorCode
	setReceivePdoCobId(uint_fast8_t index, uint32_t cobId)
	{
		auto& rpdo = Device::receivePdos_[index];
		return rpdo.setCOBId(cobId);
	}
};

}  // namespace modm_canopen

#endif  // CANOPEN_RECEIVE_PDO_CONFIGURATOR_HPP
