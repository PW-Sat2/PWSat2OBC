#ifndef SRC_COMMANDS_COMMANDS_H_
#define SRC_COMMANDS_COMMANDS_H_

#include <cstdint>

void PingHandler(std::uint16_t argc, char* argv[]);
void EchoHandler(std::uint16_t argc, char* argv[]);
void JumpToTimeHandler(std::uint16_t argc, char* argv[]);
void CurrentTimeHandler(std::uint16_t argc, char* argv[]);
void AdvanceTimeHandler(std::uint16_t argc, char* argv[]);
void SendFrameHandler(std::uint16_t argc, char* argv[]);
void GetFramesCountHandler(std::uint16_t argc, char* argv[]);
void ReceiveFrameHandler(std::uint16_t argc, char* argv[]);
void CommandPauseComm(std::uint16_t argc, char* argv[]);
void CommReset(std::uint16_t argc, char* argv[]);
void CommGetTelemetry(std::uint16_t argc, char* argv[]);
void CommSetBaudRate(std::uint16_t argc, char* argv[]);
void CommSetIdleState(std::uint16_t argc, char* argv[]);
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

#endif
