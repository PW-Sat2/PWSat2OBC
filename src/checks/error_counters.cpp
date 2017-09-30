#include "antenna/driver.h"
#include "camera/camera_low_level.h"
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
        devices::comm::CommObject,                 // 0
        devices::eps::EPSDriver,                   // 1
        devices::rtc::RTCObject,                   // 2
        devices::imtq::ImtqDriver,                 // 3
        obc::storage::error_counters::N25QDriver1, // 4
        obc::storage::error_counters::N25QDriver2, // 5
        obc::storage::error_counters::N25QDriver3, // 6
        devices::n25q::RedundantN25QDriver,        // 7
        devices::fm25w::RedundantFM25WDriver,      // 8
        devices::payload::PayloadDriver,           // 9
        devices::camera::LowLevelCameraDriver,     // 10
        devices::suns::SunSDriver,                 // 11
        antenna_error_counters::PrimaryChannel,    // 12
        antenna_error_counters::SecondaryChannel   // 13
        >;

    static_assert(Everything::IsOk, "Device IDs for error counting must be unique");
}
