#ifndef LIBS_STATE_FWD_HPP
#define LIBS_STATE_FWD_HPP

#pragma once

namespace state
{
    class AntennaConfiguration;
    class AntennaState;
    class TimeState;
    class TimeCorrectionConfiguration;
    class BootState;
    class SailState;

    struct NoTrackingStatePolicy;
    class StateTrackingPolicy;
    template <typename StatePolicy, typename... Parts> class LockablePersistentState;

    typedef LockablePersistentState<StateTrackingPolicy, //
        AntennaConfiguration,                            //
        TimeState,                                       //
        TimeCorrectionConfiguration,                     //
        BootState,                                       //
        SailState                                        //
        >
        SystemPersistentState;
}

class SystemState;

#endif
