#include <cstdlib>
#include <cstring>
#include "mission.h"
#include "obc_access.hpp"
#include "state/antenna/AntennaConfiguration.hpp"
#include "state/struct.h"
#include "state/time/TimeCorrectionConfiguration.hpp"
#include "state/time/TimeState.hpp"
#include "terminal/terminal.h"

using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::duration_cast;

enum class StateType
{
    Invalid,
    Antenna,
    TimeState,
    TimeConfig,
};

static StateType Parse(const char* name)
{
    if (strcmp(name, "antenna") == 0)
    {
        return StateType::Antenna;
    }
    else if (strcmp(name, "time_state") == 0)
    {
        return StateType::TimeState;
    }
    else if (strcmp(name, "time_config") == 0)
    {
        return StateType::TimeConfig;
    }
    else
    {
        return StateType::Invalid;
    }
}

static bool StateGet(int argc, char* argv[])
{
    if (argc < 1)
    {
        GetTerminal().Puts("Usage: state get [antenna|time_state|time_config]");
        return false;
    }

    const auto type = Parse(argv[0]);
    switch (type)
    {
        case StateType::Antenna:
        {
            state::AntennaConfiguration antennaConfiguration;
            if (!Mission.GetState().PersistentState.Get(antennaConfiguration))
            {
                GetTerminal().Puts("Can't get state::AntennaConfiguration");
                return false;
            }

            GetTerminal().Printf("%d\n", static_cast<int>(antennaConfiguration.IsDeploymentDisabled()));
            return true;
        }

        case StateType::TimeState:
        {
            state::TimeState timeState;
            if (!Mission.GetState().PersistentState.Get(timeState))
            {
                GetTerminal().Puts("Can't get state::TimeState");
                return false;
            }

            GetTerminal().Printf("%u %u\n",
                static_cast<unsigned int>(duration_cast<seconds>(timeState.LastMissionTime()).count()),
                static_cast<unsigned int>(duration_cast<seconds>(timeState.LastExternalTime()).count()));
            return true;
        }

        case StateType::TimeConfig:
        {
            state::TimeCorrectionConfiguration timeCorrectionConfiguration;
            if (!Mission.GetState().PersistentState.Get(timeCorrectionConfiguration))
            {
                GetTerminal().Puts("Can't get state::TimeCorrectionConfiguration");
                return false;
            }

            GetTerminal().Printf("%d %d\n",
                static_cast<int>(timeCorrectionConfiguration.MissionTimeFactor()),
                static_cast<int>(timeCorrectionConfiguration.ExternalTimeFactor()));
            return true;
        }

        case StateType::Invalid:
        default:
            GetTerminal().Puts("Usage: state get [antenna|time_state|time_config]");
            return false;
    }
}

static bool StateSet(int argc, char* argv[])
{
    if (argc < 1)
    {
        GetTerminal().Puts("Usage: state set [antenna|time_state|time_config]");
        return false;
    }

    auto& persistentState = Mission.GetState().PersistentState;
    const auto type = Parse(argv[0]);
    switch (type)
    {
        case StateType::Antenna:
        {
            if (argc < 2)
            {
                GetTerminal().Puts("Usage: state set antenna <deployment_disabled>");
                return false;
            }

            char* tail;
            bool value = strtol(argv[1], &tail, 10) != 0;
            if (!persistentState.Set(state::AntennaConfiguration(value)))
            {
                GetTerminal().Puts("Can't set state::AntennaConfiguration");
                return false;
            }

            return true;
        }

        case StateType::TimeState:
        {
            if (argc < 3)
            {
                GetTerminal().Puts("Usage: state set time_state <mission_time> <external_time>");
                return false;
            }

            char* tail;
            std::uint32_t missionTime = strtol(argv[1], &tail, 10);
            std::uint32_t externalTime = strtol(argv[2], &tail, 10);
            if (!persistentState.Set(state::TimeState(seconds(missionTime), seconds(externalTime))))
            {
                GetTerminal().Puts("Can't set state::TimeState");
                return false;
            }

            return true;
        }

        case StateType::TimeConfig:
        {
            if (argc < 3)
            {
                GetTerminal().Puts("Usage: state set time_config <internal_factor> <external_factor>");
                return false;
            }

            char* tail;
            auto internalFactor = static_cast<std::int16_t>(strtol(argv[1], &tail, 10));
            auto externalFactor = static_cast<std::int16_t>(strtol(argv[2], &tail, 10));
            if (!persistentState.Set(state::TimeCorrectionConfiguration(internalFactor, externalFactor)))
            {
                GetTerminal().Puts("Can't set state::TimeCorrectionConfiguration");
                return false;
            }

            return true;
        }

        case StateType::Invalid:
        default:
            GetTerminal().Puts("Usage: state set [antenna|time_state|time_config]");
            return false;
    }
}

void StateCommandHandler(uint16_t argc, char* argv[])
{
    if (argc < 1)
    {
        GetTerminal().Puts("Usage: state [set|get]");
        return;
    }

    bool status = false;
    if (strcmp(argv[0], "get") == 0)
    {
        status = StateGet(--argc, ++argv);
    }
    else if (strcmp(argv[0], "set") == 0)
    {
        status = StateSet(--argc, ++argv);
    }
    else
    {
        GetTerminal().Printf("Usage: state [set|get]");
    }

    if (status)
    {
        GetTerminal().Printf("OK");
    }
}
