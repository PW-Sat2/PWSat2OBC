#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <em_gpio.h>
#include <em_leuart.h>

#include "io_map.h"
#include "leuart.h"

extern void pingHandler();
extern void ping2Handler();

typedef void (*commandHandler)(void);

typedef struct command
{
	uint8_t name[32];
	commandHandler handler;
} command;

command commands[] =
		{
				{ "ping", &pingHandler },
				{ "ping2", &ping2Handler }
		};

QueueHandle_t terminalQueue;

void handleIncomingChar(void* args)
{
	uint8_t input_buffer[32] = { 0 };
	uint32_t input_buffer_position = 0;

	while (1)
	{
		uint8_t data;

		xQueueReceive(terminalQueue, &data, portMAX_DELAY);

		if (data == 13)
		{
			input_buffer[input_buffer_position] = 0;
			input_buffer_position = 0;

			terminalHandleCommand(input_buffer);
		}
		else
		{
			leuartPutc(data);
			input_buffer[input_buffer_position++] = data;
		}
	}
}

void terminalInit()
{
	terminalQueue = xQueueCreate(32, sizeof(uint8_t));
	xTaskCreate(handleIncomingChar, "terminalIn", 1024, NULL, 4, NULL);

	leuartInit(terminalQueue);
}

void terminalHandleCommand(uint8_t* buffer)
{
	terminalSendNewLine();

	for (int i = 0; i < sizeof(commands) / sizeof(command); i++)
	{
		if (strcmp(buffer, commands[i].name) == 0)
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

