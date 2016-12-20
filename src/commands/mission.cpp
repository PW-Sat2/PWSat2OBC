#include "mission.h"
#include "antenna/antenna.h"
#include "logger/logger.h"
#include "system.h"
#include "terminal.h"

extern mission::ObcMission Mission;

void SuspendMission(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    LOG(LOG_LEVEL_INFO, "Received request to suspend automatic mission processing.");
    Mission.Suspend();
}

void ResumeMission(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    LOG(LOG_LEVEL_INFO, "Received request to resume automatic mission processing.");
    Mission.Suspend();
}

void RunMission(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    LOG(LOG_LEVEL_INFO, "Received request to suspend automatic mission processing.");
    Mission.RunOnce();
}
