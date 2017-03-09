#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "rapidcheck.h"
#pragma GCC diagnostic pop

namespace rc
{
    template <typename Enum, Enum... Values> struct ArbitraryBitmask
    {
        static auto arbitrary()
        {
            constexpr auto count = sizeof...(Values);
            std::array<Enum, count> enumValues{Values...};
            auto enabledFlagsGen = gen::container<std::vector<bool>>(count, gen::arbitrary<bool>());

            return gen::map(enabledFlagsGen, [enumValues](std::vector<bool> enabledFlags) {
                std::underlying_type_t<Enum> result = 0;

                for (decltype(enabledFlags.size()) i = 0; i < enabledFlags.size(); i++)
                {
                    if (enabledFlags[i])
                        result |= num(enumValues[i]);
                }

                return static_cast<Enum>(result);
            });
        }
    };
}
