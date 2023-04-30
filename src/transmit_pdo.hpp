#ifndef CANOPEN_TRANSMIT_PDO_HPP
#define CANOPEN_TRANSMIT_PDO_HPP

#include "pdo_common.hpp"
#include <array>
#include <modm/architecture/interface/can_message.hpp>
#include <modm/architecture/interface/clock.hpp>
#include <modm/processing/timer/timestamp.hpp>
#include <optional>
#include <variant>

namespace modm_canopen
{

struct SendOnEvent
{
	modm::PreciseDuration eventTimeout_{};
	modm::PreciseDuration inhibitTime_{};
	modm::PreciseTimestamp lastMessage_{};
	bool updated_ = false;

	void
	setValueUpdated()
	{
		updated_ = true;
	}

	bool
	send()
	{
		const auto now = modm::chrono::micro_clock::now();
		if ((now - lastMessage_) > inhibitTime_)
		{
			const bool timerEnabled = (eventTimeout_.count() != 0);
			const bool timerExpired = (now - lastMessage_) > eventTimeout_;
			if (updated_ || (timerEnabled && timerExpired))
			{
				lastMessage_ = now;
				return true;
			}
		}
		return false;
	}
};

enum class TransmitMode
{
	OnSync,
	OnEvent
};

template<typename OD>
class TransmitPdo;

template<typename OD>
modm::can::Message
createPdoMessage(const TransmitPdo<OD> &pdo, uint16_t canId);

template<typename OD>
class TransmitPdo : public PdoObject<OD>
{
public:
	void
	sync();
	void
	setValueUpdated();

	template<typename Callback>
	std::optional<modm::can::Message>
	nextMessage(Callback &&cb);

	void
	setTransmitMode(TransmitMode mode);
	// TODO: change parameter types
	SdoErrorCode
	setEventTimeout(uint16_t milliseconds);  // 0 to disable
	SdoErrorCode
	setInhibitTime(uint16_t inhibitTime_100us);
	uint16_t
	eventTimeout() const;
	uint16_t
	inhibitTime() const;

private:
	TransmitMode transmitMode_{};
	SendOnEvent sendOnEvent_{};
	bool sync_{false};

	template<typename Callback>
	std::optional<modm::can::Message>
	getMessage(Callback &&cb);

protected:
	SdoErrorCode
	validateMapping(PdoMapping mapping) override;
};

}  // namespace modm_canopen

#include "transmit_pdo_impl.hpp"

#endif  // CANOPEN_TRANSMIT_PDO_HPP
