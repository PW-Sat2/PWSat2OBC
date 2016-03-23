#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <em_gpio.h>
#include <em_leuart.h>

#include "io_map.h"
#include "drivers/leuart.h"

extern void pingHandler(uint16_t argc, char* argv[]);
extern void echoHandler(uint16_t argc, char* argv[]);

typedef void (*commandHandler)(uint16_t argc, char* argv[]);

typedef struct command
{
	char name[32];
	commandHandler handler;
} command;

command commands[] =
		{
				{ "ping", &pingHandler },
				{ "echo", &echoHandler }
		};

QueueHandle_t terminalQueue;

void handleIncomingChar(void* args)
{
	char input_buffer[32] = { 0 };
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

void parseCommandLine(char line[], char** commandName, char** arguments, uint16_t* argc)
{
 volatile char* ptr;
 char* token = strtok_r(line, " ", &ptr);

 *commandName = token;

 token = strtok_r((char*)NULL, " ", &ptr);

 while (token != NULL )
 {
  arguments[*argc] = token;
  (*argc)++;

  token = strtok_r((char*)NULL, " ", &ptr);
 }
}

void terminalHandleCommand(char* buffer)
{
	char* commandName;
	uint32_t argc = 0;
	char* args[8] = {0};

	terminalSendNewLine();

	parseCommandLine(buffer, &commandName, args, &argc);

	for (int i = 0; i < sizeof(commands) / sizeof(command); i++)
	{
		if (strcmp(commandName, commands[i].name) == 0)
		{
			commands[i].handler(argc, args);
			terminalSendNewLine();
		}
	}

	terminalSendPrefix();
}

void terminalSendNewLine(void)
{
	leuartPuts("\r\n");
}

void terminalSendPrefix(void)
{
	leuartPuts("PW-SAT2->");
}

void terminalInit()
{
	terminalQueue = xQueueCreate(32, sizeof(uint8_t));
	xTaskCreate(handleIncomingChar, "terminalIn", 1024, NULL, 4, NULL);

	leuartInit(terminalQueue);
}
