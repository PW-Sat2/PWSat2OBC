#include "comm/CommDriver.hpp"
#include "eps/eps.h"
#include "n25q/n25q.h"
#include "obc/storage/n25q.h"
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
    using Everything = VerifyUniqueness<           //
        devices::eps::EPSDriver,                   //
        devices::comm::CommObject,                 //
        obc::storage::error_counters::N25QDriver1, //
        obc::storage::error_counters::N25QDriver2, //
        obc::storage::error_counters::N25QDriver3  //
        >;

    static_assert(Everything::IsOk, "Device IDs for error counting must be unique");
}
