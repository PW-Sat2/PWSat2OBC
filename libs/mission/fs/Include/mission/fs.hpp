#ifndef MISSION_FS_HPP
#define MISSION_FS_HPP

#pragma once

#include <chrono>
#include "fs/yaffs.h"
#include "mission/base.hpp"
#include "state/struct.h"

namespace mission
{
    /**
     * @brief Task that is responsible for periodical file system synchronization.
     * @mission_task
     */
    class FileSystemTask : public Action, public RequireNotifyWhenTimeChanges
    {
      public:
        /**
         * @brief ctor.
         */
        FileSystemTask(services::fs::IYaffsDeviceOperations& deviceOperations);

        /**
         * @brief Builds action descriptor for this task.
         * @return Action descriptor
         */
        ActionDescriptor<SystemState> BuildAction();

        /**
         * @brief Event raised by main Mission Loop when mission time changes.
         * @param timeCorrection The time correction value. Positive - time has been advanced. Negative - time has been taken back.
         */
        void TimeChanged(std::chrono::milliseconds timeCorrection);

        /**
         * @brief Period of file system sync.
         */
        static constexpr std::chrono::milliseconds SyncPeriod = std::chrono::minutes(10);

      private:
        static void CreateCheckpoint(SystemState& state, void* param);
        static bool CanCreateCheckpoint(const SystemState& state, void* param);

        services::fs::IYaffsDeviceOperations& _deviceOperations;

        Option<std::chrono::milliseconds> _lastUpdate;
    };
}

#endif
