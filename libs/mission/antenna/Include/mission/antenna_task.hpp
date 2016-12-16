#ifndef LIBS_MISSION_ANTENNA_TASK_HPP
#define LIBS_MISSION_ANTENNA_TASK_HPP

#pragma once

#include "antenna_state.h"
#include "mission/base.hpp"

namespace mission
{
    namespace antenna
    {
        /**
         * @brief Mission part related to antenna deployment.
         * @ingroup mission_atenna
         */
        struct AntennaTask : public Update, public Action
        {
            /**
             * @brief ctor.
             * @param[in] driver Reference to antenna driver interface.
             */
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
