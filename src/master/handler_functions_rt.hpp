#pragma once
#include <variant>
#include <functional>
#include <optional>
#include <cstdint>
#include "../float_types.hpp"
#include "../sdo_error.hpp"

namespace modm_canopen
{
template<typename T>
using ReadFunctionRT = std::function<std::optional<T>()>;

template<typename T>
using WriteFunctionRT = std::function<SdoErrorCode(T)>;

using ReadHandlerRT =
	std::variant<std::monostate, ReadFunctionRT<uint8_t>, ReadFunctionRT<uint16_t>,
				 ReadFunctionRT<uint32_t>, ReadFunctionRT<uint64_t>, ReadFunctionRT<int8_t>,
				 ReadFunctionRT<int16_t>, ReadFunctionRT<int32_t>, ReadFunctionRT<int64_t>,
				 ReadFunctionRT<float32_t>>;

using WriteHandlerRT =
	std::variant<std::monostate, WriteFunctionRT<uint8_t>, WriteFunctionRT<uint16_t>,
				 WriteFunctionRT<uint32_t>, WriteFunctionRT<uint64_t>, WriteFunctionRT<int8_t>,
				 WriteFunctionRT<int16_t>, WriteFunctionRT<int32_t>, WriteFunctionRT<int64_t>,
				 WriteFunctionRT<float32_t>>;
}  // namespace modm_canopen