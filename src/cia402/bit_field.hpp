#pragma once
#include <cstdint>
namespace modm_canopen
{
template<typename T>
constexpr inline uint16_t
toBitField()
{
	return 0;
}

template<typename T, T first, T... rest>
constexpr inline uint16_t
toBitField()
{
	return toBitField<T, rest...>() | (uint16_t)first;
}

template<typename T, T... bits>
inline bool
bitFieldHasSet(uint16_t word)
{
	uint16_t mask = toBitField<T, bits...>();
	return (word & mask) == mask;
}

template<typename T, T... bits>
inline bool
bitFieldHasUnset(uint16_t word)
{
	uint16_t mask = toBitField<T, bits...>();
	return (word & mask) == 0;
}
}  // namespace modm_canopen