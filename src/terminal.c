#include <string.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <em_gpio.h>
#include <em_leuart.h>

#include "io_map.h"
#include "leuart/leuart.h"
#include "system.h"
#include "terminal.h"
#include "commands/commands.h"
#include "swo/swo.h"

typedef void (*commandHandler)(uint16_t argc, char* argv[]);

typedef struct
{
	char name[32];
	commandHandler handler;
} command;

static const command commands[] =
		{
				{ "ping", &pingHandler },
				{ "echo", &echoHandler },
				{ "jumpToTime", &jumpToTimeHandler },
				{ "currentTime", &currentTimeHandler }
		};

QueueHandle_t terminalQueue;

static void parseCommandLine(char line[], char** commandName, char** arguments, uint16_t* argc, uint8_t maxArgsCount)
{
	*argc = 0;

	char* ptr;
	char* token = strtok_r(line, " ", &ptr);

	*commandName = token;

	token = strtok_r(NULL, " ", &ptr);

	while (token != NULL && *argc < maxArgsCount)
	{
		arguments[*argc] = token;
		(*argc)++;

		token = strtok_r(NULL, " ", &ptr);
	}
}

void terminalSendNewLine(void)
{
	leuartPuts("\n");
}

static void terminalSendPrefix(void)
{
	leuartPuts(">");
}

void terminalPrintf(const char * text, ...)
{
	va_list args;
	va_start(args, text);

	leuartvPrintf(text, args);

	va_end(args);
}

static void terminalHandleCommand(char* buffer)
{
	char* commandName;
	uint16_t argc = 0;
	char* args[8] = { 0 };

	terminalSendNewLine();

	parseCommandLine(buffer, &commandName, args, &argc, COUNT_OF(args));

	for (size_t i = 0; i < COUNT_OF(commands); i++)
	{
		if (strcmp(commandName, commands[i].name) == 0)
		{
			commands[i].handler(argc, args);
			terminalSendNewLine();
		}
	}

	terminalSendPrefix();
}

static void handleIncomingChar(void* args)
{
	UNREFERENCED_PARAMETER(args);

	char input_buffer[32] = { 0 };
	uint32_t input_buffer_position = 0;

	while (1)
	{
		uint8_t data = 0;

		xQueueReceive(terminalQueue, &data, portMAX_DELAY);

		if (data == '\n')
		{
			input_buffer[input_buffer_position] = 0;
			input_buffer_position = 0;

			terminalHandleCommand(input_buffer);
		}
		else if (input_buffer_position < sizeof(input_buffer) - 1)
		{
			leuartPutc(data);
			input_buffer[input_buffer_position++] = data;
		}
	}
}

void terminalInit(void)
{
	terminalQueue = xQueueCreate(32, sizeof(uint8_t));

	if(terminalQueue != NULL)
	{
		if(xTaskCreate(handleIncomingChar, "terminalIn", 1024, NULL, 4, NULL) != pdPASS)
		{
			leuartInit(terminalQueue);
		}
		else
		{
			SwoPuts("Error. Cannot create terminalIn thread.");
		}
	}
	else
	{
		SwoPuts("Error. Cannot create terminalQueue.");
	}
}
