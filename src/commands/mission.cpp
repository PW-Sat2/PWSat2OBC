#include "mission.h"
#include "antenna/antenna.h"
#include "logger/logger.h"
#include "obc/experiments.hpp"
#include "obc_access.hpp"
#include "system.h"
#include "terminal.h"
#include "terminal/terminal.h"

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
    Mission.Resume();
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
        GetTerminal().Puts("set_fibo_iterations <iterations>");
        return;
    }

    std::uint16_t iters = atoi(argv[0]);

    GetExperiments().Get<experiment::fibo::FibonacciExperiment>().Iterations(iters);
}

void RequestExperiment(std::uint16_t argc, char* argv[])
{
    if (argc != 1)
    {
        GetTerminal().Puts("request_experiment <experimentType>");
        return;
    }

    std::uint16_t expType = atoi(argv[0]);

    GetExperiments().ExperimentsController.RequestExperiment(gsl::narrow_cast<experiments::ExperimentCode>(expType));
}

void AbortExperiment(std::uint16_t argc, char* argv[])
{
    UNUSED(argc, argv);

    GetExperiments().ExperimentsController.AbortExperiment();
}

void ExperimentInfo(std::uint16_t argc, char* argv[])
{
    UNUSED(argc, argv);
    auto state = GetExperiments().ExperimentsController.CurrentState();

    if (state.RequestedExperiment.HasValue)
        GetTerminal().Printf("Requested\t%d\n", state.RequestedExperiment.Value);
    else
        GetTerminal().Puts("Requested\tNone\n");

    if (state.CurrentExperiment.HasValue)
        GetTerminal().Printf("Current\t%d\n", state.CurrentExperiment.Value);
    else
        GetTerminal().Puts("Current\tNone\n");

    if (state.LastStartResult.HasValue)
        GetTerminal().Printf("LastStartResult\t%d\n", num(state.LastStartResult.Value));
    else
        GetTerminal().Puts("LastStartResult\tNone\n");

    if (state.LastIterationResult.HasValue)
        GetTerminal().Printf("LastIterationResult\t%d\n", num(state.LastIterationResult.Value));
    else
        GetTerminal().Puts("LastIterationResult\tNone\n");

    GetTerminal().Printf("IterationCounter\t%ld\n", state.IterationCounter);
}
