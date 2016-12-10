#ifndef LIBS_MISSION_INCLUDE_MISSION_TIME_HPP_
#define LIBS_MISSION_INCLUDE_MISSION_TIME_HPP_

#pragma once

#include "mission/base.hpp"
#include "state/struct.h"
#include "time/timer.h"

namespace mission
{
    class TimeTask : public Update
    {
      public:
        TimeTask(TimeProvider& timeProvider);

        UpdateDescriptor<SystemState> BuildUpdate();

      private:
        static UpdateResult UpdateProc(SystemState& state, void* param);

        TimeProvider& provider;
    };
}

#endif /* LIBS_MISSION_INCLUDE_MISSION_TIME_HPP_ */
