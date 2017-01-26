#ifndef LIBS_MISSION_INCLUDE_MISSION_ADCS_MISSION_H_
#define LIBS_MISSION_INCLUDE_MISSION_ADCS_MISSION_H_

#pragma once

#include <cstdint>
#include "adcs/adcs.hpp"
#include "mission/base.hpp"
#include "state/struct.h"

namespace mission
{
    namespace adcs
    {
        /**
         * @defgroup MissionADCS System state support for ADCS
         *
         * @ingroup Mission
         * @{
         */
        /**
         * @brief ADCS descriptors
         */
        class AdcsPrimaryTask : public Update, Action
        {
          public:
            AdcsPrimaryTask(::adcs::IAdcsCoordinator& adcsCoordinator);

            /**
             * @brief Returns adcs primary detumbling action descriptor.
             *
             * @returns Action descriptor that runs primary adcs detumbling algorithm.
             */
            ActionDescriptor<SystemState> BuildAction();

            /**
             * @brief Returns adcs status update descriptor.
             *
             * @returns Update descriptor that runs adcs status update process.
             */
            UpdateDescriptor<SystemState> BuildUpdate();

          private:
            /**
             * @brief Procedure that verifies whether the adcs built-in detumbling process should be executed.
             * @param[in] state Pointer to global satellite state.
             * @param[in] param Pointer to the deployment condition private context. This pointer should point
             * at the object of AdcsPrimaryTask type.
             *
             * @return True if the deployment action should be performed, false otherwise.
             */
            static bool AdcsEnablePrimaryDetumblingCondition(const SystemState& state, void* param);

            /**
             * @brief This procedure is deployment action entry point.
             *
             * This procedure runs the antenna deployment process.
             * @param[in] state Pointer to global satellite state.
             * @param[in] param Pointer to the deployment condition private context. This pointer should point
             * at the object of AdcsPrimaryTask type.
             */
            static void AdcsEnablePrimaryDetumbling(const SystemState& state, void* param);

            std::uint8_t retryCount;
            bool finished;
            ::adcs::IAdcsCoordinator& coordinator;
        };

        /** @} */
    }
}

#endif /* LIBS_MISSION_INCLUDE_MISSION_ADCS_MISSION_H_ */
