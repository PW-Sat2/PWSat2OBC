#include "mission.h"
#include "antenna/antenna.h"
#include "logger/logger.h"
#include "obc.h"
#include "system.h"
#include "terminal.h"

extern mission::ObcMission Mission;

void SuspendMission(std::uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    LOG(LOG_LEVEL_INFO, "Received request to suspend automatic mission processing.");
    Mission.Suspend();
}

void ResumeMission(std::uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    LOG(LOG_LEVEL_INFO, "Received request to resume automatic mission processing.");
    Mission.Suspend();
}

void RunMission(std::uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    LOG(LOG_LEVEL_INFO, "Received request to run mission processing once");
    Mission.RequestSingleIteration();
}

void SetFiboIterations(std::uint16_t argc, char* argv[])
{
    if (argc != 1)
    {
        Main.terminal.Puts("set_fibo_iterations <iterations>");
        return;
    }

    std::uint16_t iters = atoi(argv[0]);

    Main.Fibo.Iterations(iters);
}

void RequestExperiment(std::uint16_t argc, char* argv[])
{
    if (argc != 1)
    {
        Main.terminal.Puts("request_experiment <experimentType>");
        return;
    }

    std::uint16_t expType = atoi(argv[0]);

    Main.ExperimentsController.RequestExperiment(static_cast<mission::experiments::Experiment>(expType));
}
