#include "antenna/antenna.h"
#include <cstring>
#include "antenna/driver.h"
#include "antenna/telemetry.hpp"
#include "logger/logger.h"
#include "obc_access.hpp"
#include "system.h"
#include "terminal.h"
#include "terminal/terminal.h"
#include "utils.h"

using namespace std::chrono_literals;

// static bool GetChannel(const char* name, AntennaChannel* channel)
// {
//     if (strcmp(name, "primary") == 0)
//     {
//         *channel = ANTENNA_PRIMARY_CHANNEL;
//     }
//     else if (strcmp(name, "backup") == 0)
//     {
//         *channel = ANTENNA_BACKUP_CHANNEL;
//     }
//     else
//     {
//         return false;
//     }

//     return true;
// }

// void PrintValue(int value, const char* name)
// {
//     GetTerminal().Printf("%s: '%d'\n", name, value);
// }

void AntennaGetTelemetry(std::uint16_t /*argc*/, char* /*argv*/ [])
{
    // devices::antenna::AntennaTelemetry telemetry;
    // GetAntennaDriver().GetTelemetry(telemetry);
    // auto& counts1 = telemetry.GetActivationCounts(ANTENNA_PRIMARY_CHANNEL);
    // auto& counts2 = telemetry.GetActivationCounts(ANTENNA_BACKUP_CHANNEL);
    // auto& times1 = telemetry.GetActivationTimes(ANTENNA_PRIMARY_CHANNEL);
    // auto& times2 = telemetry.GetActivationTimes(ANTENNA_BACKUP_CHANNEL);

    // PrintValue(counts1.GetActivationCount(ANTENNA1_ID), "Antenna 1 activation count (primary)");
    // PrintValue(counts1.GetActivationCount(ANTENNA2_ID), "Antenna 2 activation count (primary)");
    // PrintValue(counts1.GetActivationCount(ANTENNA3_ID), "Antenna 3 activation count (primary)");
    // PrintValue(counts1.GetActivationCount(ANTENNA4_ID), "Antenna 4 activation count (primary)");

    // PrintValue(counts2.GetActivationCount(ANTENNA1_ID), "Antenna 1 activation count (secondary)");
    // PrintValue(counts2.GetActivationCount(ANTENNA2_ID), "Antenna 2 activation count (secondary)");
    // PrintValue(counts2.GetActivationCount(ANTENNA3_ID), "Antenna 3 activation count (secondary)");
    // PrintValue(counts2.GetActivationCount(ANTENNA4_ID), "Antenna 4 activation count (secondary)");

    // PrintValue(times1.GetActivationTime(ANTENNA1_ID).count(), "Antenna 1 activation time (primary)");
    // PrintValue(times1.GetActivationTime(ANTENNA2_ID).count(), "Antenna 2 activation time (primary)");
    // PrintValue(times1.GetActivationTime(ANTENNA3_ID).count(), "Antenna 3 activation time (primary)");
    // PrintValue(times1.GetActivationTime(ANTENNA4_ID).count(), "Antenna 4 activation time (primary)");

    // PrintValue(times2.GetActivationTime(ANTENNA1_ID).count(), "Antenna 1 activation time (secondary)");
    // PrintValue(times2.GetActivationTime(ANTENNA2_ID).count(), "Antenna 2 activation time (secondary)");
    // PrintValue(times2.GetActivationTime(ANTENNA3_ID).count(), "Antenna 3 activation time (secondary)");
    // PrintValue(times2.GetActivationTime(ANTENNA4_ID).count(), "Antenna 4 activation time (secondary)");
}

void AntennaReset(std::uint16_t argc, char* argv[])
{
    // AntennaChannel channel;
    // if (argc != 1 || !GetChannel(argv[0], &channel))
    // {
    //     GetTerminal().Puts("antenna_reset [primary|backup]\n");
    //     return;
    // }

    // const OSResult result = GetAntennaDriver().Reset(channel);
    // if (OS_RESULT_SUCCEEDED(result))
    // {
    //     GetTerminal().Puts("Done");
    // }
    // else
    // {
    //     GetTerminal().Printf("Unable to reset antenna. Status: '%d'", num(result));
    // }
}
