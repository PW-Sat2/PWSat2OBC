#include <em_gpio.h>
#include "em_leuart.h"

#include "io_map.h"
#include "leuart.h"

void pingHandler()
{
	leuartPuts("pong");
}

void ping2Handler()
{
	leuartPuts("pong2");
}

typedef void (*commandHandler)(void);

typedef struct command {
	uint8_t name[32];
	commandHandler handler;
} command;

command commands[] =
{
		{ "ping" , &pingHandler },
		{ "ping2" , &ping2Handler }
};

void terminalHandleCommand(uint8_t* buffer)
{
	terminalSendNewLine();

	for(int i = 0; i < sizeof(commands)/sizeof(command); i++)
	{
		if(strcmp(buffer, commands[i].name) == 0)
		{
			commands[i].handler();
			terminalSendNewLine();
		}
	}

	terminalSendPrefix();
}

void terminalSendNewLine()
{
	leuartPuts("\r\n");
}

void terminalSendPrefix()
{
	leuartPuts("PW-SAT2->");
}

