#include "comm/CommDriver.hpp"
#include "eps/eps.h"
#include "fm25w/fm25w.hpp"
#include "imtq/imtq.h"
#include "n25q/n25q.h"
#include "obc/storage/n25q.h"
#include "rtc/rtc.hpp"
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
        devices::eps::EPSDriver,                   // 5
        devices::comm::CommObject,                 // 1
        devices::imtq::ImtqDriver,                 // 6
        devices::fm25w::RedundantFM25WDriver,      // 11
        devices::rtc::RTCObject,                   // 7
        obc::storage::error_counters::N25QDriver1, // 8
        obc::storage::error_counters::N25QDriver2, // 9
        obc::storage::error_counters::N25QDriver3  // 10
        >;

    static_assert(Everything::IsOk, "Device IDs for error counting must be unique");
}
