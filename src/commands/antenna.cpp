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

static void SendResult(OSResult result)
{
    GetTerminal().Printf("%d", num(result));
}

static bool GetChannel(const char* name, AntennaChannel* channel)
{
    if (strcmp(name, "primary") == 0)
    {
        *channel = ANTENNA_PRIMARY_CHANNEL;
    }
    else if (strcmp(name, "backup") == 0)
    {
        *channel = ANTENNA_BACKUP_CHANNEL;
    }
    else
    {
        return false;
    }

    return true;
}

static bool GetAntenna(const char* name, AntennaId* antenna)
{
    if (strcmp(name, "auto") == 0)
    {
        *antenna = ANTENNA_AUTO_ID;
    }
    else if (strcmp(name, "1") == 0)
    {
        *antenna = ANTENNA1_ID;
    }
    else if (strcmp(name, "2") == 0)
    {
        *antenna = ANTENNA2_ID;
    }
    else if (strcmp(name, "3") == 0)
    {
        *antenna = ANTENNA3_ID;
    }
    else if (strcmp(name, "4") == 0)
    {
        *antenna = ANTENNA4_ID;
    }
    else
    {
        return false;
    }

    return true;
}

void AntennaDeploy(std::uint16_t argc, char* argv[])
{
    AntennaChannel channel;
    AntennaId antenna;
    if (                                  //
        (argc != 2 && argc != 3) ||       //
        !GetChannel(argv[0], &channel) || //
        !GetAntenna(argv[1], &antenna)    //
        )
    {
        GetTerminal().Puts("antenna_deploy [primary|backup] [auto|1|2|3|4] [override]\n");
        return;
    }

    const bool override = (argc > 2) && (strcmp(argv[2], "override") == 0);
    const OSResult result = GetAntennaDriver().DeployAntenna(channel,
        antenna,
        10s,
        override //
        );
    SendResult(result);
    return;
}

void AntennaCancelDeployment(std::uint16_t argc, char* argv[])
{
    AntennaChannel channel;
    if (                               //
        argc < 1 ||                    //
        !GetChannel(argv[0], &channel) //
        )
    {
        GetTerminal().Puts("antenna_cancel [primary|backup]\n");
        return;
    }

    GetAntennaDriver().FinishDeployment(channel);
}

void AntennaGetDeploymentStatus(std::uint16_t argc, char* argv[])
{
    AntennaChannel channel;
    if (                               //
        argc < 1 ||                    //
        !GetChannel(argv[0], &channel) //
        )
    {
        GetTerminal().Puts("antenna_get_status [primary|backup]\n");
        return;
    }

    AntennaDeploymentStatus deploymentStatus;
    const OSResult status = GetAntennaDriver().GetDeploymentStatus(channel, &deploymentStatus);
    if (OS_RESULT_FAILED(status))
    {
        SendResult(status);
    }
    else
    {
        GetTerminal().Printf("Status: %d\n", num(status));
        GetTerminal().Printf("Deployment status: %d %d %d %d\n",
            ToInt(deploymentStatus.DeploymentStatus[0]), //
            ToInt(deploymentStatus.DeploymentStatus[1]), //
            ToInt(deploymentStatus.DeploymentStatus[2]), //
            ToInt(deploymentStatus.DeploymentStatus[3]));

        GetTerminal().Printf("Deployment active: %d %d %d %d\n",
            ToInt(deploymentStatus.IsDeploymentActive[0]), //
            ToInt(deploymentStatus.IsDeploymentActive[1]), //
            ToInt(deploymentStatus.IsDeploymentActive[2]), //
            ToInt(deploymentStatus.IsDeploymentActive[3]));

        GetTerminal().Printf("Deployment time reached: %d %d %d %d\n",
            ToInt(deploymentStatus.DeploymentTimeReached[0]), //
            ToInt(deploymentStatus.DeploymentTimeReached[1]), //
            ToInt(deploymentStatus.DeploymentTimeReached[2]), //
            ToInt(deploymentStatus.DeploymentTimeReached[3]));

        GetTerminal().Printf("Ignoring switches: %d\n", ToInt(deploymentStatus.IgnoringDeploymentSwitches));
        GetTerminal().Printf("Independent burn: %d\n", ToInt(deploymentStatus.IsIndependentBurnActive));
        GetTerminal().Printf("Armed: %d\n", ToInt(deploymentStatus.DeploymentSystemArmed));
    }
}

void PrintValue(int value, const char* name)
{
    GetTerminal().Printf("%s: '%d'\n", name, value);
}

void AntennaGetTelemetry(std::uint16_t /*argc*/, char* /*argv*/ [])
{
    devices::antenna::AntennaTelemetry telemetry;
    GetAntennaDriver().GetTelemetry(telemetry);
    auto& counts1 = telemetry.GetActivationCounts(ANTENNA_PRIMARY_CHANNEL);
    auto& counts2 = telemetry.GetActivationCounts(ANTENNA_BACKUP_CHANNEL);
    auto& times1 = telemetry.GetActivationTimes(ANTENNA_PRIMARY_CHANNEL);
    auto& times2 = telemetry.GetActivationTimes(ANTENNA_BACKUP_CHANNEL);

    PrintValue(counts1.GetActivationCount(ANTENNA1_ID), "Antenna 1 activation count (primary)");
    PrintValue(counts1.GetActivationCount(ANTENNA2_ID), "Antenna 2 activation count (primary)");
    PrintValue(counts1.GetActivationCount(ANTENNA3_ID), "Antenna 3 activation count (primary)");
    PrintValue(counts1.GetActivationCount(ANTENNA4_ID), "Antenna 4 activation count (primary)");

    PrintValue(counts2.GetActivationCount(ANTENNA1_ID), "Antenna 1 activation count (secondary)");
    PrintValue(counts2.GetActivationCount(ANTENNA2_ID), "Antenna 2 activation count (secondary)");
    PrintValue(counts2.GetActivationCount(ANTENNA3_ID), "Antenna 3 activation count (secondary)");
    PrintValue(counts2.GetActivationCount(ANTENNA4_ID), "Antenna 4 activation count (secondary)");

    PrintValue(times1.GetActivationTime(ANTENNA1_ID).count(), "Antenna 1 activation time (primary)");
    PrintValue(times1.GetActivationTime(ANTENNA2_ID).count(), "Antenna 2 activation time (primary)");
    PrintValue(times1.GetActivationTime(ANTENNA3_ID).count(), "Antenna 3 activation time (primary)");
    PrintValue(times1.GetActivationTime(ANTENNA4_ID).count(), "Antenna 4 activation time (primary)");

    PrintValue(times2.GetActivationTime(ANTENNA1_ID).count(), "Antenna 1 activation time (secondary)");
    PrintValue(times2.GetActivationTime(ANTENNA2_ID).count(), "Antenna 2 activation time (secondary)");
    PrintValue(times2.GetActivationTime(ANTENNA3_ID).count(), "Antenna 3 activation time (secondary)");
    PrintValue(times2.GetActivationTime(ANTENNA4_ID).count(), "Antenna 4 activation time (secondary)");
}

void AntennaReset(std::uint16_t argc, char* argv[])
{
    AntennaChannel channel;
    if (argc != 1 || !GetChannel(argv[0], &channel))
    {
        GetTerminal().Puts("antenna_reset [primary|backup]\n");
        return;
    }

    const OSResult result = GetAntennaDriver().Reset(channel);
    if (OS_RESULT_SUCCEEDED(result))
    {
        GetTerminal().Puts("Done");
    }
    else
    {
        GetTerminal().Printf("Unable to reset antenna. Status: '%d'", num(result));
    }
}
