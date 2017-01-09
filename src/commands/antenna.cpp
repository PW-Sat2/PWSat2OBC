#include "antenna/antenna.h"
#include "logger/logger.h"
#include "obc.h"
#include "system.h"
#include "terminal.h"
#include "utils.h"

static void SendResult(OSResult result)
{
    Main.terminal.Printf("%d", result);
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

void AntennaDeploy(uint16_t argc, char* argv[])
{
    AntennaChannel channel;
    AntennaId antenna;
    if (                                  //
        (argc != 2 && argc != 3) ||       //
        !GetChannel(argv[0], &channel) || //
        !GetAntenna(argv[1], &antenna)    //
        )
    {
        Main.terminal.Puts("antenna_deploy [primary|backup] [auto|1|2|3|4] [override]\n");
        return;
    }

    const bool override = (argc > 2) && (strcmp(argv[2], "override") == 0);
    const OSResult result = Main.antennaDriver.DeployAntenna(&Main.antennaDriver,
        channel,
        antenna,
        std::chrono::seconds(10),
        override //
        );
    SendResult(result);
    return;
}

void AntennaCancelDeployment(uint16_t argc, char* argv[])
{
    AntennaChannel channel;
    if (                               //
        argc < 1 ||                    //
        !GetChannel(argv[0], &channel) //
        )
    {
        Main.terminal.Puts("antenna_cancel [primary|backup]\n");
        return;
    }

    Main.antennaDriver.FinishDeployment(&Main.antennaDriver, channel);
}

void AntennaGetDeploymentStatus(uint16_t argc, char* argv[])
{
    AntennaChannel channel;
    if (                               //
        argc < 1 ||                    //
        !GetChannel(argv[0], &channel) //
        )
    {
        Main.terminal.Puts("antenna_get_status [primary|backup]\n");
        return;
    }

    AntennaDeploymentStatus deploymentStatus;
    const OSResult status = Main.antennaDriver.GetDeploymentStatus(&Main.antennaDriver, channel, &deploymentStatus);
    if (OS_RESULT_FAILED(status))
    {
        SendResult(status);
    }
    else
    {
        Main.terminal.Printf("%d %d %d %d %d %d %d %d %d %d %d\n",
            status,
            ToInt(deploymentStatus.DeploymentStatus[0]),        //
            ToInt(deploymentStatus.DeploymentStatus[1]),        //
            ToInt(deploymentStatus.DeploymentStatus[2]),        //
            ToInt(deploymentStatus.DeploymentStatus[3]),        //
            ToInt(deploymentStatus.IsDeploymentActive[0]),      //
            ToInt(deploymentStatus.IsDeploymentActive[1]),      //
            ToInt(deploymentStatus.IsDeploymentActive[2]),      //
            ToInt(deploymentStatus.IsDeploymentActive[3]),      //
            ToInt(deploymentStatus.IgnoringDeploymentSwitches), //
            ToInt(deploymentStatus.DeploymentSystemArmed)       //
            );
    }
}
