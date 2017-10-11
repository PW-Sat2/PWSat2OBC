#ifndef LIBS_MISSION_MEMORY_INCLUDE_MISSION_MEMORY_RECOVERY_HPP_
#define LIBS_MISSION_MEMORY_INCLUDE_MISSION_MEMORY_RECOVERY_HPP_

#include "mission/base.hpp"
#include "state/fwd.hpp"

namespace mission
{
    /**
     * @brief Mission task for performing SRAM recovery
     * @ingroup mission
     * @mission_task
     */
    class MemoryRecoveryTask : public mission::Action
    {
      public:
        /** @brief Type of procedure perfomring recovery */
        using RecoveryProc = void (*)();

        /**
         * @brief Ctor
         * @param performRecovery Pointer to procedure for perfoming recovery
         */
        MemoryRecoveryTask(RecoveryProc performRecovery);

        /**
         * @brief Builds action descriptor
         * @return Action descriptor
         */
        ActionDescriptor<SystemState> BuildAction();

      private:
        /**
         * @brief Action callback
         * @param state Unused
         * @param param Pointer to recovery procedure
         */
        static void Action(SystemState& state, void* param);

        /** @brief Pointer to recovery procedure */
        RecoveryProc _performRecovery;
    };
}

#endif /* LIBS_MISSION_MEMORY_INCLUDE_MISSION_MEMORY_RECOVERY_HPP_ */
