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
	// TODO: OnSync
};

template<typename OD>
class ReceivePdo : public PdoObject<OD>
{
public:
	template<typename Callback>
	void
	processMessage(const modm::can::Message &message, Callback &&cb);

protected:
	virtual SdoErrorCode
	validateMapping(PdoMapping mapping) override;
};
}  // namespace modm_canopen
#include "receive_pdo_impl.hpp"

#endif  // CANOPEN_RECEIVE_PDO_HPP
