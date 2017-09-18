#ifndef LIBS_MISSION_BEACON_BEACON_HPP
#define LIBS_MISSION_BEACON_BEACON_HPP

#pragma once

#include <chrono>
#include "base/fwd.hpp"
#include "mission/base.hpp"
#include "state/struct.h"

namespace mission
{
    /**
     * @defgroup MissionBeacon Beacon management
     *
     * This module contains components responsible for coordinating beacon transmission and updating its contents.
     * @ingroup mission
     * @{
     */
    /**
     * @brief Beacon state handler.
     * @mission_task
     *
     * This task is responsible for setting & updating the beacon that is being constantly send via the communication module.
     */
    class BeaconUpdate : public Action
    {
      public:
        /**
         * @brief ctor.
         * @param unused Unused argument
         */
        BeaconUpdate(std::uint8_t unused);

        /**
         * @brief Returns action descriptor for this task.
         * @returns Action descriptor that runs beacon update task.
         */
        ActionDescriptor<SystemState> BuildAction();

        /**
         * @brief Sets handle to beacon task
         * @param handle Beacon task handle
         */
        void BeaconTaskHandle(OSTaskHandle handle);

      private:
        /**
         * @brief Procedure that verifies whether the beacon should be set/updated.
         * @param[in] state Pointer to global satellite state.
         * @param[in] param Pointer to the deployment condition private context. This pointer should point
         * at the object of BeaconUpdate type.
         *
         * @return True if the beacon should be updated, false otherwise.
         */
        static bool ShouldUpdateBeacon(const SystemState& state, void* param);

        /**
         * @brief Updates current beacons.
         * @param[in] state Reference to global mission state.
         * @param[in] param Current execution context.
         */
        static void Run(SystemState& state, void* param);

        /** @brief Beacon task handle */
        OSTaskHandle beaconTaskHandle;

        /** @brief Flag indicating whether beacon is already enabled */
        bool isBeaconEnabled;
    };
}

#endif
