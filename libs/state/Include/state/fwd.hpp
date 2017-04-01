#ifndef LIBS_STATE_FWD_HPP
#define LIBS_STATE_FWD_HPP

#pragma once

namespace state
{
    class AntennaConfiguration;
    class AntennaState;
    class TimeState;

    template <typename... Parts> class PersistentState;

    typedef PersistentState<AntennaConfiguration, TimeState> SystemPersistentState;
}

class SystemState;

#endif
