#pragma once
#include "object_dictionary_common.hpp"

namespace modm_canopen
{

namespace detail
{

template<Address address>
struct missing_read_handler
{
	static_assert(address == Address{}, "Read handler not registered for at least one object");
};

template<Address address>
struct missing_write_handler
{
	static_assert(address == Address{}, "Write handler not registered for at least one object");
};

}  // namespace detail

}  // namespace modm_canopen