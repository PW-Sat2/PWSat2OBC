#ifndef SRC_COMMANDS_COMMANDS_H_
#define SRC_COMMANDS_COMMANDS_H_

#include <stdint.h>

void PingHandler(uint16_t argc, char* argv[]);
void EchoHandler(uint16_t argc, char* argv[]);
void JumpToTimeHandler(uint16_t argc, char* argv[]);
void CurrentTimeHandler(uint16_t argc, char* argv[]);
void SendFrameHandler(uint16_t argc, char* argv[]);
void GetFramesCountHandler(uint16_t argc, char* argv[]);
void ReceiveFrameHandler(uint16_t argc, char* argv[]);
void CommandPauseComm(uint16_t argc, char* argv[]);
void OBCGetState(uint16_t argc, char* argv[]);
void FSListFiles(uint16_t argc, char* argv[]);
void FSWriteFile(uint16_t argc, char* argv[]);
void FSReadFile(uint16_t argc, char* argv[]);
void CommandByTerminal(uint16_t argc, char* args[]);
void I2CTestCommandHandler(uint16_t argc, char* argv[]);

void AntennaDeploy(uint16_t argc, char* argv[]);
void AntennaCancelDeployment(uint16_t argc, char* argv[]);
void AntennaGetDeploymentStatus(uint16_t argc, char* argv[]);

#endif
