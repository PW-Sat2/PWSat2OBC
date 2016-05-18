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

#endif
