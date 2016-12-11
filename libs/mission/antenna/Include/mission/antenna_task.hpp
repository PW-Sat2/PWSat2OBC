#ifndef LIBS_MISSION_ANTENNA_TASK_HPP
#define LIBS_MISSION_ANTENNA_TASK_HPP

#pragma once

#include "antenna_state.h"
#include "mission/base.hpp"

namespace mission
{
    namespace antenna
    {
        struct AntennaTask final : public Update, public Action
        {
            AntennaTask(AntennaDriver& driver);
            /**
             * @brief Returns antenna deployment action descriptor.
             *
             * @returns Action descriptor that runs antenna deployment process.
             */
            ActionDescriptor<SystemState> BuildAction();

            /**
             * @brief Returns antenna deployment update descriptor.
             *
             * @returns Update descriptor that runs antenna deployment update process.
             */
            UpdateDescriptor<SystemState> BuildUpdate();

            /**
             * @brief State of the antenna mission deployment task.
             */
            AntennaMissionState state;
        };
    }
}

#endif
