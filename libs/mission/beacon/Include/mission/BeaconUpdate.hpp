#ifndef LIBS_MISSION_BEACON_BEACON_HPP
#define LIBS_MISSION_BEACON_BEACON_HPP

#pragma once

#include <chrono>
#include "comm/Beacon.hpp"
#include "comm/comm.hpp"
#include "mission/base.hpp"
#include "state/struct.h"
#include "telecommunication/downlink.h"

namespace mission
{
    class BeaconUpdate : public Action
    {
      public:
        BeaconUpdate(devices::comm::IBeaconController& beaconController);

        /**
         * @brief Returns action descriptor for this task.
         * @returns Action descriptor that runs beacon update task.
         */
        ActionDescriptor<SystemState> BuildAction();

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
        static void Run(const SystemState& state, void* param);

        void UpdateBeacon(const SystemState& state);

        devices::comm::Beacon GenerateBeacon(const SystemState& state);

        devices::comm::IBeaconController* controller;

        std::chrono::milliseconds lastBeaconUpdate;

        telecommunication::downlink::DownlinkFrame frame;
    };
}

#endif
