#include "antenna/antenna.h"
#include "logger/logger.h"
#include "obc.h"
#include "system.h"
#include "terminal.h"

static void SendResult(OSResult result)
{
    TerminalPrintf(&Main.terminal, "%d", result);
}

void AntennaChangeDeploymentSystemState(uint16_t argc, char* argv[])
{
    LOGF(LOG_LEVEL_INFO, "Received arm antenna command with %d arguments. ", (int)argc);

    if (argc != 1)
    {
        TerminalPuts(&Main.terminal, "antenna [arm|disarm]\n");
        return;
    }

    OSResult result = -1;
    if (strcmp(argv[0], "arm") == 0)
    {
        result = Main.antennaDriver.ArmDeploymentSystem(&Main.antennaDriver);
    }
    else if (strcmp(argv[0], "disarm") == 0)
    {
        result = Main.antennaDriver.DisarmDeploymentSystem(&Main.antennaDriver);
    }

    LOGF(LOG_LEVEL_INFO, "Finished arm antenna command with %d arguments status: '%d'. ", (int)argc, result);
}

void AntennaDeploy(uint16_t argc, char* argv[])
{
    if (argc < 1)
    {
        TerminalPuts(&Main.terminal, "antenna_deploy [auto|1|2|3|4] [override]\n");
        return;
    }

    if (strcmp(argv[0], "auto") == 0)
    {
        const OSResult result = Main.antennaDriver.InitializeAutomaticDeployment(&Main.antennaDriver);
        SendResult(result);
        return;
    }

    OSResult (*procedure)(struct AntennaDriver * driver, AntennaId antennaId, TimeSpan timeout);
    AntennaId antenna = ANTENNA1;
    if (argc > 1)
    {
        if (strcmp(argv[1], "override") == 0)
        {
            procedure = Main.antennaDriver.DeployAntennaOverride;
        }
        else
        {
            procedure = Main.antennaDriver.DeployAntenna;
        }
    }

    if (strcmp(argv[0], "1") == 0)
    {
        antenna = ANTENNA1;
    }
    else if (strcmp(argv[0], "2") == 0)
    {
        antenna = ANTENNA2;
    }
    else if (strcmp(argv[0], "3") == 0)
    {
        antenna = ANTENNA3;
    }
    else if (strcmp(argv[0], "4") == 0)
    {
        antenna = ANTENNA4;
    }

    const OSResult result = procedure(&Main.antennaDriver, antenna, TimeSpanFromSeconds(10));
    SendResult(result);
}

void AntennaCancelDeployment(uint16_t argc, char* argv[])
{
    Main.antennaDriver.CancelAntennaDeployment(&Main.antennaDriver);
}
