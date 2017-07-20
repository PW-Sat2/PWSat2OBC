#include "comm/CommDriver.hpp"
#include "eps/eps.h"
#include "fm25w/fm25w.hpp"
#include "imtq/imtq.h"
#include "n25q/n25q.h"
#include "obc/storage/n25q.h"
#include "payload/payload.h"
#include "rtc/rtc.hpp"
#include "suns/suns.hpp"
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
        devices::imtq::ImtqDriver,                 //
        devices::fm25w::RedundantFM25WDriver,      //
        devices::rtc::RTCObject,                   //
        drivers::payload::PayloadDriver,           //
        obc::storage::error_counters::N25QDriver1, //
        obc::storage::error_counters::N25QDriver2, //
        obc::storage::error_counters::N25QDriver3, //
        devices::n25q::RedundantN25QDriver,        //
        devices::suns::SunSDriver                  //
        >;

    static_assert(Everything::IsOk, "Device IDs for error counting must be unique");
}
