#ifndef CANOPEN_RECEIVE_PDO_HPP
#define CANOPEN_RECEIVE_PDO_HPP

#include "pdo_common.hpp"
#include <array>
#include <modm/architecture/interface/can_message.hpp>

namespace modm_canopen
{

enum class ReceiveMode
{
	Async,
	OnSync
};

template<typename OD>
class ReceivePdo : public PdoObject<OD>
{
public:
	template<typename Callback>
	void
	processMessage(const modm::can::Message &message, Callback &&cb);

	template<typename Device>
	void update(bool justLeftSyncWindow);

	bool
	setTransmitMode(uint8_t mode);

protected:
	virtual SdoErrorCode
	validateMapping(PdoMapping mapping) override;

	bool received_{false};
	uint8_t passedSyncs_{0};
};
}  // namespace modm_canopen
#include "receive_pdo_impl.hpp"

#endif  // CANOPEN_RECEIVE_PDO_HPP
