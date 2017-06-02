#include "comm/CommDriver.hpp"
#include "eps/eps.h"
#include "imtq/imtq.h"
#include "traits.hpp"

namespace
{
    /**
     * @brief Verifies uniqueness of all declared error counters
     */
    template <typename... Components> struct VerifyUniqueness
    {
        /** @brief Value indicating whether error counters are declared properly */
        static constexpr bool IsOk = AreTypesUnique<typename Components::ErrorCounter...>::value;
    };

    /** @brief Typedef with all declared error counters */
    using Everything = VerifyUniqueness< //
        devices::eps::EPSDriver,         //
        devices::comm::CommObject,       //
        devices::imtq::ImtqDriver        //
        >;

    static_assert(Everything::IsOk, "Device IDs for error counting must be unique");
}
