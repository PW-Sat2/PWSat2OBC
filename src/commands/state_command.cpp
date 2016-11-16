#include <stdint.h>
#include <stdlib.h>
#include "adcs/adcs.h"
#include "mission.h"
#include "obc.h"
#include "system.h"

void CommandByTerminal(uint16_t argc, char* args[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(args);

    int16_t cmd = atoi(args[0]);

    SetTerminalCommand((TerminalCommand)cmd);
}
