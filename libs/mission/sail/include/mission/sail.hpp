#ifndef LIBS_MISSION_INCLUDE_MISSION_SAIL_H_
#define LIBS_MISSION_INCLUDE_MISSION_SAIL_H_

#pragma once

#include "mission/base.hpp"
#include "state/struct.h"

namespace mission
{
    class SailTask : public Action, public Update
    {
      public:
        SailTask() = default;

        SailTask(bool initial);

        ActionDescriptor<SystemState> BuildAction();

        UpdateDescriptor<SystemState> BuildUpdate();

        void SetState(bool newState);

        bool CurrentState() const noexcept;

      private:
        static bool CanOpenSail(const SystemState& state, void* param);

        static void OpenSail(const SystemState& state, void* param);

        static UpdateResult UpdateProc(SystemState& state, void* param);

        bool state;
    };

    inline void SailTask::SetState(bool newState)
    {
        this->state = newState;
    }

    inline bool SailTask::CurrentState() const noexcept
    {
        return this->state;
    }
}
#endif /* LIBS_MISSION_INCLUDE_MISSION_SAIL_H_ */
