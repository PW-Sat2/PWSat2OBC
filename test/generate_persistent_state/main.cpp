#include <array>
#include <cstdio>
#include <unistd.h>
#include "base/Writer.h"
#include "state/LockablePersistentState.hpp"
#include "state/StatePolicies.hpp"
#include "state/adcs/AdcsState.hpp"
#include "state/antenna/AntennaConfiguration.hpp"
#include "state/antenna/AntennaState.hpp"
#include "state/comm/MessageState.hpp"
#include "state/fdir/ErrorCountersState.hpp"
#include "state/fwd.hpp"
#include "state/sail/SailState.hpp"
#include "state/time/TimeCorrectionConfiguration.hpp"
#include "state/time/TimeState.hpp"
#include "system.h"
#include "utils.h"

extern "C" {
extern void initialise_monitor_handles(void);
extern void __libc_init_array(void);
extern int kill(pid_t, int);
}

using namespace std::chrono_literals;
using namespace state;

void FillState(SystemPersistentState& state)
{
    state.Set(AntennaConfiguration(true));
    state.Set(TimeState(0x11223344556677ms, 0x28776655443322ms));
    state.Set(TimeCorrectionConfiguration(1122, 3344));
    state.Set(SailState(SailOpeningState::OpeningStopped));
    state.Set(AdcsState(false));
}

void WritePersistentStateToFile()
{
    auto f = fopen("persistent_state", "wb");

    SystemPersistentState state;

    FillState(state);

    std::array<std::uint8_t, decltype(state)::Size()> buf;
    buf.fill(0);

    Writer writer(buf);
    state.Write(writer);

    fwrite(buf.data(), 1, buf.size(), f);

    fclose(f);
}

int main()
{
    __libc_init_array();

    initialise_monitor_handles();

    WritePersistentStateToFile();

    kill(-1, 0);

    return 0;
}
