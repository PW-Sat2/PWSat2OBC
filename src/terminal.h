#ifndef SRC_TERMINAL_H_
#define SRC_TERMINAL_H_

void terminalSendNewLine(void);
void terminalHandleCommand(uint8_t* buffer);
void terminalInit(void);

#endif
