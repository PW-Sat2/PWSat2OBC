#ifndef SRC_COMMANDS_COMMANDS_H_
#define SRC_COMMANDS_COMMANDS_H_

void PingHandler(uint16_t argc, char* argv[]);
void EchoHandler(uint16_t argc, char* argv[]);
void JumpToTimeHandler(uint16_t argc, char* argv[]);
void CurrentTimeHandler(uint16_t argc, char* argv[]);

void TakeRAWPhotoHandler(uint16_t argc, char* argv[]);
void TakeJPEGPhotoHandler(uint16_t argc, char* argv[]);

#endif
