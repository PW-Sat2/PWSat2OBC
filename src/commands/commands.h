#ifndef SRC_COMMANDS_COMMANDS_H_
#define SRC_COMMANDS_COMMANDS_H_

#include <cstdint>

void PingHandler(std::uint16_t argc, char* argv[]);
void IdHandler(std::uint16_t argc, char* argv[]);
void EchoHandler(std::uint16_t argc, char* argv[]);
void BootParamsCommand(std::uint16_t argc, char* argv[]);
void JumpToTimeHandler(std::uint16_t argc, char* argv[]);
void CurrentTimeHandler(std::uint16_t argc, char* argv[]);
void AdvanceTimeHandler(std::uint16_t argc, char* argv[]);
void Comm(std::uint16_t argc, char* argv[]);
void WaitForOBCInitialization(std::uint16_t argc, char* argv[]);
void FSListFiles(std::uint16_t argc, char* argv[]);
void FSWriteFile(std::uint16_t argc, char* argv[]);
void FSReadFile(std::uint16_t argc, char* argv[]);
void RemoveFile(uint16_t argc, char* argv[]);
void MountList(std::uint16_t argc, char* argv[]);
void MakeDirectory(std::uint16_t argc, char* argv[]);
void EraseFlash(std::uint16_t argc, char* argv[]);
void SyncFS(std::uint16_t argc, char* argv[]);
void CommandByTerminal(std::uint16_t argc, char* args[]);
void I2CTestCommandHandler(std::uint16_t argc, char* argv[]);
void HeapInfoCommand(std::uint16_t argc, char* argv[]);

void AntennaDeploy(std::uint16_t argc, char* argv[]);
void AntennaCancelDeployment(std::uint16_t argc, char* argv[]);
void AntennaGetDeploymentStatus(std::uint16_t argc, char* argv[]);
void AntennaGetTelemetry(std::uint16_t argc, char* argv[]);
void AntennaReset(std::uint16_t argc, char* argv[]);

void HeapInfo(std::uint16_t argc, char* argv[]);

void TaskListCommand(std::uint16_t argc, char* argv[]);
void CompileInfo(std::uint16_t argc, char* argv[]);
void SuspendMission(std::uint16_t argc, char* argv[]);
void ResumeMission(std::uint16_t argc, char* argv[]);
void RunMission(std::uint16_t argc, char* argv[]);
void SetFiboIterations(std::uint16_t argc, char* argv[]);

void RequestExperiment(std::uint16_t argc, char* argv[]);
void AbortExperiment(std::uint16_t argc, char* argv[]);
void ExperimentInfo(std::uint16_t argc, char* argv[]);

void DMAInfo(std::uint16_t argc, char* argv[]);
void ResetHandler(std::uint16_t argc, char* argv[]);

void RTCTest(std::uint16_t argc, char* argv[]);

void ImtqDriver(uint16_t argc, char* argv[]);

void GyroDriver(uint16_t argc, char* argv[]);

void FRAM(std::uint16_t argc, char* argv[]);

void TestExternalFlash(std::uint16_t argc, char* argv[]);

void Temp(std::uint16_t argc, char* argv[]);

void EPSCommand(std::uint16_t argc, char* argv[]);

void WatchdogCommand(std::uint16_t argc, char* argv[]);
void Hang(std::uint16_t argc, char* argv[]);

void StateCommandHandler(uint16_t argc, char* argv[]);

void ErrorCountersCommand(std::uint16_t argc, char* argv[]);

void Scrubbing(std::uint16_t argc, char* argv[]);

void BootSettingsCommand(std::uint16_t argc, char* argv[]);

void PayloadDriver(std::uint16_t argc, char* argv[]);

void MemoryCommand(std::uint16_t argc, char* argv[]);

void RunlevelCommand(std::uint16_t argc, char* argv[]);

void SailCommand(std::uint16_t argc, char* argv[]);

#endif
