#pragma once
namespace modm_canopen
{

template<class... Ts>
struct overloaded : Ts...
{
	using Ts::operator()...;
};  // i hate this

}  // namespace modm_canopen