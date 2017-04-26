#include <gsl/span>

#include "commands/commands.h"
#include "obc.h"
#include "terminal/terminal.h"

static const TerminalCommandDescription commands[] = {
    {"ping", PingHandler},
    {"echo", EchoHandler},
    {"jumpToTime", JumpToTimeHandler},
    {"currentTime", CurrentTimeHandler},
    {"wait_for_init", WaitForOBCInitialization},
    {"listFiles", FSListFiles},
    {"writeFile", FSWriteFile},
    {"readFile", FSReadFile},
    {"mountList", MountList},
    {"mkdir", MakeDirectory},
    {"erase", EraseFlash},
    {"sync_fs", SyncFS},
    {"i2c", I2CTestCommandHandler},
    {"antenna_deploy", AntennaDeploy},
    {"antenna_cancel", AntennaCancelDeployment},
    {"antenna_get_status", AntennaGetDeploymentStatus},
    {"heap", HeapInfo},
    {"tasklist", TaskListCommand},
    {"heap", HeapInfoCommand},
    {"advance_time", AdvanceTimeHandler},
    {"compile_info", CompileInfo},
    {"suspend_mission", SuspendMission},
    {"resume_mission", ResumeMission},
    {"run_mission", RunMission},
    {"set_fibo_iterations", SetFiboIterations},
    {"request_experiment", RequestExperiment},
    {"abort_experiment", AbortExperiment},
    {"experiment_info", ExperimentInfo},
    {"dma", DMAInfo},
    {"imtq", ImtqDriver},
    {"comm", Comm},
    {"antenna_get_telemetry", AntennaGetTelemetry},
    {"antenna_reset", AntennaReset},
    {"reset", ResetHandler},
    {"rtc", RTCTest},
    {"rm", RemoveFile},
    {"fram", FRAM},
    {"testflash", TestExternalFlash},
    {"temp", Temp},
    {"gyro", GyroDriver},
    {"eps", EPSCommand},
    {"wdog", WatchdogCommand},
    {"hang", Hang},
    {"state", StateCommandHandler},
    {"error_counters", ErrorCountersCommand},
};

void InitializeTerminal(void)
{
    Main.terminal.Initialize();
    Main.terminal.SetCommandList(gsl::span<const TerminalCommandDescription>(commands));
}
