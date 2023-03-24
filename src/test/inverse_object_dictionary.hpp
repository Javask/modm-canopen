#pragma once
#include "object_dictionary_common.hpp"

namespace modm_canopen
{
template<typename OD>
class inverse
{
private:
	constexpr static inline auto
	makeInverse()
	{
		ConstexprMapBuilder<Address, Entry, OD::map.size()> builder;
		for (auto it = OD::map.begin(); it != OD::map.end(); it++)
		{
			auto inverted = it->second;
			switch (inverted.accessType)
			{
				case AccessType::ReadOnly:
					inverted.accessType = AccessType::WriteOnly;
					break;
				case AccessType::ReadWriteReadPdo:
					inverted.accessType = AccessType::ReadWriteWritePdo;
					break;
				case AccessType::WriteOnly:
					inverted.accessType = AccessType::ReadOnly;
					break;
				case AccessType::ReadWriteWritePdo:
					inverted.accessType = AccessType::ReadWriteReadPdo;
					break;
				default:
					break;
			}
			builder.insert(it->first, inverted);
		}
		return builder.buildMap();
	}

public:
	static constexpr inline ConstexprMap<Address, Entry, OD::map.size()> map = makeInverse();
};

}  // namespace modm_canopen