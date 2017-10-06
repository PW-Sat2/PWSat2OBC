#ifndef LIBS_STATE_FWD_HPP
#define LIBS_STATE_FWD_HPP

#pragma once

namespace state
{
    class AntennaConfiguration;
    class AntennaState;
    class TimeState;
    class TimeCorrectionConfiguration;
    class SailState;
    class ErrorCountersConfigState;
    class MessageState;
    class AdcsState;

    struct NoTrackingStatePolicy;
    class StateTrackingPolicy;
    template <typename StatePolicy, typename... Parts> class LockablePersistentState;

    typedef LockablePersistentState<StateTrackingPolicy, //
        AntennaConfiguration,                            //
        TimeState,                                       //
        TimeCorrectionConfiguration,                     //
        SailState,                                       //
        ErrorCountersConfigState,                        //
        AdcsState,                                       //
        MessageState                                     //
        >
        SystemPersistentState;
}

class SystemState;

#endif
