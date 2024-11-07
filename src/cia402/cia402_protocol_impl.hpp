#ifndef CANOPEN_CIA402_PROTOCOL_HPP
#error "Do not include this file directly, use cia402_protocol.hpp instead"
#endif

namespace modm_canopen::cia402
{

template<uint8_t Axis>
void
CiA402<Axis>::setError(){
	status_.startFaultReaction();
}

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
{
	if(status_.wasChanged()){
		MODM_LOG_DEBUG << "Status was changed!" << modm::endl;
		Device::setValueChanged(CiA402Objects<Axis>::StatusWord);
	}
	switch (status_.state())
	{
		case State::QuickStopActive:
			// Do quickstop
			break;
		case State::OperationEnabled:
			// Do OperatingMode update
			break;
		case State::DisableReactionActive:
			// Slow down depending on disable operation code
			status_.setReactionDone();
			break;
		case State::ShutdownReactionActive:
			// Slow down depending on shutdown operation code
			status_.setReactionDone();
			break;
		case State::HaltReactionActive:
			// Slow down depending on halt operation code
			//status_.setReactionDone();
			break;
		case State::FaultReactionActive:
			// Slow down depending on fault operation code
			status_.setReactionDone();
			break;
		default:
			break;
	}
}

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

	map.template setReadHandler<CiA402Objects<Axis>::ModeOfOperation>(
		+[]() { return int8_t(demandedMode_); });

	map.template setWriteHandler<CiA402Objects<Axis>::ModeOfOperation>(+[](int8_t value) {
		if (isSupported((OperatingMode)value))
		{
			auto newMode = (static_cast<OperatingMode>(value));
			if (demandedMode_ != newMode)
			{
				demandedMode_ = newMode;
				MODM_LOG_INFO << "Set demanded operating mode to "
							  << modm_canopen::cia402::operatingModeToString(demandedMode_)
							  << modm::endl;
			}
			return SdoErrorCode::NoError;
		} else
		{
			return SdoErrorCode::InvalidValue;
		}
	});

	map.template setReadHandler<CiA402Objects<Axis>::ModeOfOperationDisplay>(
		+[]() { return int8_t(displayedMode_); });

	map.template setReadHandler<CiA402Objects<Axis>::ControlWord>(
		+[]() { return status_.control(); });

	map.template setWriteHandler<CiA402Objects<Axis>::ControlWord>(+[](uint16_t value) {
		if (!status_.update(value)) return SdoErrorCode::InvalidValue;
		return SdoErrorCode::NoError;
	});

	map.template setReadHandler<CiA402Objects<Axis>::StatusWord>(
		+[]() { return status_.status(); });

	map.template setReadHandler<CiA402Objects<Axis>::SupportedDriveModes>(
		+[]() { return supportedModesBitfield_; });

	map.template setReadHandler<CiA402Objects<Axis>::QuickStopOptionCode>(
		+[]() { return std::to_underlying(quickStopCode_); });
	map.template setWriteHandler<CiA402Objects<Axis>::QuickStopOptionCode>(+[](int16_t value) {
		if (value < 0 || value > 8) return SdoErrorCode::InvalidValue;
		quickStopCode_ = OptionCode(value);
		return SdoErrorCode::NoError;
	});

	map.template setReadHandler<CiA402Objects<Axis>::ShutdownOptionCode>(
		+[]() { return std::to_underlying(shutdownCode_); });
	map.template setWriteHandler<CiA402Objects<Axis>::ShutdownOptionCode>(+[](int16_t value) {
		if (value != 0 && value != 1) return SdoErrorCode::InvalidValue;
		shutdownCode_ = OptionCode(value);
		return SdoErrorCode::NoError;
	});

	map.template setReadHandler<CiA402Objects<Axis>::DisableOperationOptionCode>(
		+[]() { return std::to_underlying(disableCode_); });
	map.template setWriteHandler<CiA402Objects<Axis>::DisableOperationOptionCode>(
		+[](int16_t value) {
			if (value != 0 && value != 1) return SdoErrorCode::InvalidValue;
			disableCode_ = OptionCode(value);
			return SdoErrorCode::NoError;
		});

	map.template setReadHandler<CiA402Objects<Axis>::HaltOptionCode>(
		+[]() { return std::to_underlying(haltCode_); });
	map.template setWriteHandler<CiA402Objects<Axis>::HaltOptionCode>(+[](int16_t value) {
		if (value < 0 || value > 4) return SdoErrorCode::InvalidValue;
		haltCode_ = OptionCode(value);
		return SdoErrorCode::NoError;
	});

	map.template setReadHandler<CiA402Objects<Axis>::FaultReactionOptionCode>(
		+[]() { return std::to_underlying(faultCode_); });
	map.template setWriteHandler<CiA402Objects<Axis>::FaultReactionOptionCode>(+[](int16_t value) {
		if (value < 0 || value > 4) return SdoErrorCode::InvalidValue;
		faultCode_ = OptionCode(value);
		return SdoErrorCode::NoError;
	});
}
}  // namespace modm_canopen::cia402