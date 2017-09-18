#ifndef LIBS_MISSION_BEACON_BEACON_HPP
#define LIBS_MISSION_BEACON_BEACON_HPP

#pragma once

#include <chrono>
#include "base/fwd.hpp"
#include "comm/Beacon.hpp"
#include "comm/comm.hpp"
#include "mission/base.hpp"
#include "state/struct.h"
#include "telecommunication/downlink.h"
#include "telemetry/fwd.hpp"

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
    class BeaconUpdate : public Action, public RequireNotifyWhenTimeChanges
    {
      public:
        /**
         * @brief ctor.
         * @param[in] arguments Beacon update task dependencies.
         */
        BeaconUpdate(std::pair<devices::comm::IBeaconController&, IHasState<telemetry::TelemetryState>&> arguments);

        /**
         * @brief Returns action descriptor for this task.
         * @returns Action descriptor that runs beacon update task.
         */
        ActionDescriptor<SystemState> BuildAction();

        /**
         * @brief Event raised by main Mission Loop when mission time changes.
         * @param timeCorrection The time correction value. Positive - time has been advanced. Negative - time has been taken back.
         */
        void TimeChanged(std::chrono::milliseconds timeCorrection);

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

        /**
         * @brief Updates current beacons.
         * @param[in] state Reference to global mission state.
         */
        void UpdateBeacon(const SystemState& state);

        /**
         * @brief This procedure is responsible for generation beacon from current system state.
         * @return Object that contains new beacon definition or empty object in case of failures.
         */
        Option<devices::comm::Beacon> GenerateBeacon();

        /**
         * @brief Beacon hardware controller.
         */
        devices::comm::IBeaconController* controller;

        IHasState<telemetry::TelemetryState>* telemetryState;

        /**
         * @brief Time of last successful beacon update.
         */
        std::chrono::milliseconds lastBeaconUpdate;

        /**
         * @brief Beacon frame builder.
         */
        telecommunication::downlink::RawFrame frame;
    };
}

#endif
