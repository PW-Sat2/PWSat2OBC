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
void OBCGetState(std::uint16_t argc, char* argv[]);
void FSListFiles(std::uint16_t argc, char* argv[]);
void FSWriteFile(std::uint16_t argc, char* argv[]);
void FSReadFile(std::uint16_t argc, char* argv[]);
void CommandByTerminal(std::uint16_t argc, char* args[]);
void I2CTestCommandHandler(std::uint16_t argc, char* argv[]);
void HeapInfoCommand(std::uint16_t argc, char* argv[]);

void AntennaDeploy(std::uint16_t argc, char* argv[]);
void AntennaCancelDeployment(std::uint16_t argc, char* argv[]);
void AntennaGetDeploymentStatus(std::uint16_t argc, char* argv[]);

void TaskListCommand(std::uint16_t argc, char* argv[]);

void CompileInfo(std::uint16_t argc, char* argv[]);

#endif
