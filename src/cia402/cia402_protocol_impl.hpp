#ifndef CANOPEN_CIA402_PROTOCOL_HPP
#error "Do not include this file directly, use cia402_protocol.hpp instead"
#endif

namespace modm_canopen::cia402
{
bool
CiA402::isSupported(OperatingMode mode)
{
	if ((int)mode >= 0)
		return supportedModesBitfield_ & (1 << (((int)mode) - 1));
	else
		return supportedModesBitfield_ & (1 << (32 - (int)mode));
}

template<typename Device, typename MessageCallback>
void
CiA402::update(MessageCallback &&)
{}

template<typename Device, typename MessageCallback>
void
CiA402::processMessage(const modm::can::Message &, MessageCallback &&)
{}

template<typename ObjectDictionary>
constexpr void
CiA402::registerHandlers(HandlerMap<ObjectDictionary> &map)
{
	CiA402Factors::registerHandlers(map);

	map.template setReadHandler<CiA402Objects::ModeOfOperation>(+[]() { return int8_t(mode_); });

	map.template setReadHandler<CiA402Objects::ModeOfOperationDisplay>(
		+[]() { return int8_t(mode_); });

	map.template setWriteHandler<CiA402Objects::ModeOfOperation>(+[](int8_t value) {
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

	map.template setReadHandler<CiA402Objects::ControlWord>(+[]() { return control_.value(); });

	map.template setWriteHandler<CiA402Objects::ControlWord>(+[](uint16_t value) {
		control_.update(value);
		status_.update(control_);
		return SdoErrorCode::NoError;
	});

	map.template setReadHandler<CiA402Objects::StatusWord>(+[]() { return status_.status(); });
	map.template setReadHandler<CiA402Objects::SupportedDriveModes>(
		+[]() { return supportedModesBitfield_; });
}
}  // namespace modm_canopen::cia402