#include <cstring>
#include "mission.h"
#include "obc.h"
#include "state/antenna/AntennaConfiguration.hpp"
#include "state/struct.h"
#include "state/time/TimeCorrectionConfiguration.hpp"
#include "state/time/TimeState.hpp"

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
        Main.terminal.Puts("Usage: state get [antenna|time_state|time_config]");
        return false;
    }

    const auto type = Parse(argv[0]);
    switch (type)
    {
        case StateType::Antenna:
        {
            const auto antennaState = Mission.GetState().PersistentState.Get<state::AntennaConfiguration>();
            Main.terminal.Printf("%d\n", static_cast<int>(antennaState.IsDeploymentDisabled()));
            return true;
        }

        case StateType::TimeState:
        {
            const auto object = Mission.GetState().PersistentState.Get<state::TimeState>();
            Main.terminal.Printf("%u %u\n",
                static_cast<unsigned int>(duration_cast<seconds>(object.LastMissionTime()).count()),
                static_cast<unsigned int>(duration_cast<seconds>(object.LastExternalTime()).count()));
            return true;
        }

        case StateType::TimeConfig:
        {
            const auto config = Mission.GetState().PersistentState.Get<state::TimeCorrectionConfiguration>();
            Main.terminal.Printf("%d %d\n", static_cast<int>(config.MissionTimeFactor()), static_cast<int>(config.ExternalTimeFactor()));
            return true;
        }

        case StateType::Invalid:
        default:
            Main.terminal.Puts("Usage: state get [antenna|time_state|time_config]");
            return false;
    }
}

static bool StateSet(int argc, char* argv[])
{
    if (argc < 1)
    {
        Main.terminal.Puts("Usage: state set [antenna|time_state|time_config]");
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
                Main.terminal.Puts("Usage: state set antenna <deployment_disabled>");
                return false;
            }

            char* tail;
            bool value = strtol(argv[1], &tail, 10) != 0;
            persistentState.Set(state::AntennaConfiguration(value));
            return true;
        }

        case StateType::TimeState:
        {
            if (argc < 3)
            {
                Main.terminal.Puts("Usage: state set time_state <mission_time> <external_time>");
                return false;
            }

            char* tail;
            std::uint32_t missionTime = strtol(argv[1], &tail, 10);
            std::uint32_t externalTime = strtol(argv[2], &tail, 10);
            persistentState.Set(state::TimeState(seconds(missionTime), seconds(externalTime)));
            return true;
        }

        case StateType::TimeConfig:
        {
            if (argc < 3)
            {
                Main.terminal.Puts("Usage: state set time_config <internal_factor> <external_factor>");
                return false;
            }

            char* tail;
            auto internalFactor = static_cast<std::int16_t>(strtol(argv[1], &tail, 10));
            auto externalFactor = static_cast<std::int16_t>(strtol(argv[2], &tail, 10));
            persistentState.Set(state::TimeCorrectionConfiguration(internalFactor, externalFactor));
            return true;
        }

        case StateType::Invalid:
        default:
            Main.terminal.Puts("Usage: state set [antenna|time_state|time_config]");
            return false;
    }
}

void StateCommandHandler(uint16_t argc, char* argv[])
{
    if (argc < 1)
    {
        Main.terminal.Puts("Usage: state [set|get]");
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
        Main.terminal.Printf("Usage: state [set|get]");
    }

    if (status)
    {
        Main.terminal.Printf("OK");
    }
}
