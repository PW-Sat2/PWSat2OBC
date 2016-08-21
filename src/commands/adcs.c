#include "mission/adcs.h"
#include <stdint.h>
#include <stdlib.h>
#include "obc.h"
#include "system.h"

void CommandADCS(int argc, char* args[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(args);

    int16_t cmd = atoi(args[0]);

    Main.adcs.Command(&Main.adcs, (ADCSCommand)cmd);
}
