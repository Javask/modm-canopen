#ifndef CANOPEN_CIA402_PROTOCOL_HPP
#error "Do not include this file directly, use cia402_protocol.hpp instead"
#endif

namespace modm_canopen::cia402
{
template<uint8_t Axis>
bool
CiA402<Axis>::isSupported(OperatingMode mode)
{
	if ((int)mode >= 0)
		return supportedModesBitfield_ & (1 << (((int)mode) - 1));
	else
		return supportedModesBitfield_ & (1 << (32 - (int)mode));
}

template<uint8_t Axis>
template<typename Device, typename MessageCallback>
void
CiA402<Axis>::update(MessageCallback &&)
{}

template<uint8_t Axis>
template<typename Device, typename MessageCallback>
void
CiA402<Axis>::processMessage(const modm::can::Message &, MessageCallback &&)
{}

template<uint8_t Axis>
template<typename ObjectDictionary>
constexpr void
CiA402<Axis>::registerHandlers(HandlerMap<ObjectDictionary> &map)
{
	CiA402Factors<Axis>::registerHandlers(map);

	map.template setReadHandler<CiA402Objects<Axis>::ModeOfOperation>(+[]() { return int8_t(mode_); });

	map.template setReadHandler<CiA402Objects<Axis>::ModeOfOperationDisplay>(
		+[]() { return int8_t(mode_); });

	map.template setWriteHandler<CiA402Objects<Axis>::ModeOfOperation>(+[](int8_t value) {
		if (isSupported((OperatingMode)value))
		{
			auto newMode = (static_cast<OperatingMode>(value));
			if (mode_ != newMode)
			{
				mode_ = newMode;
				MODM_LOG_INFO << "Set operating mode to "
							  << modm_canopen::cia402::operatingModeToString(mode_) << modm::endl;
			}
			return SdoErrorCode::NoError;
		} else
		{
			return SdoErrorCode::InvalidValue;
		}
	});

	map.template setReadHandler<CiA402Objects<Axis>::ControlWord>(+[]() { return control_.value(); });

	map.template setWriteHandler<CiA402Objects<Axis>::ControlWord>(+[](uint16_t value) {
		control_.update(value);
		status_.update(control_);
		return SdoErrorCode::NoError;
	});

	map.template setReadHandler<CiA402Objects<Axis>::StatusWord>(+[]() { return status_.status(); });
	map.template setReadHandler<CiA402Objects<Axis>::SupportedDriveModes>(
		+[]() { return supportedModesBitfield_; });
}
}  // namespace modm_canopen::cia402