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
        result = Main.antennaMiniport.ArmDeploymentSystem(&Main.antennaMiniport, (I2CBus*)&Main.I2CFallback, ANTENNA_PRIMARY_CHANNEL);
    }
    else if (strcmp(argv[0], "disarm") == 0)
    {
        result = Main.antennaMiniport.DisarmDeploymentSystem(&Main.antennaMiniport, (I2CBus*)&Main.I2CFallback, ANTENNA_PRIMARY_CHANNEL);
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
        const OSResult result = Main.antennaDriver.DeployAntenna(&Main.antennaDriver,
            ANTENNA_PRIMARY_CHANNEL,
            AUTO_ID,
            TimeSpanFromSeconds(10),
            false //
            );
        SendResult(result);
        return;
    }

    AntennaId antenna = ANTENNA1_ID;
    const bool override = (argc > 1) && (strcmp(argv[1], "override") == 0);
    if (strcmp(argv[0], "1") == 0)
    {
        antenna = ANTENNA1_ID;
    }
    else if (strcmp(argv[0], "2") == 0)
    {
        antenna = ANTENNA2_ID;
    }
    else if (strcmp(argv[0], "3") == 0)
    {
        antenna = ANTENNA3_ID;
    }
    else if (strcmp(argv[0], "4") == 0)
    {
        antenna = ANTENNA4_ID;
    }

    const OSResult result = Main.antennaDriver.DeployAntenna(&Main.antennaDriver,
        ANTENNA_PRIMARY_CHANNEL,
        antenna,
        TimeSpanFromSeconds(10),
        override //
        );

    SendResult(result);
}

void AntennaCancelDeployment(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    Main.antennaDriver.FinishDeployment(&Main.antennaDriver, ANTENNA_PRIMARY_CHANNEL);
}
