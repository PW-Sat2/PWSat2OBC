#include <string.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <em_gpio.h>
#include <em_leuart.h>

#include "commands/commands.h"
#include "io_map.h"
#include "leuart/leuart.h"
#include "logger/logger.h"
#include "swo/swo.h"
#include "system.h"
#include "terminal.h"

typedef void (*commandHandler)(uint16_t argc, char* argv[]);

typedef struct
{
    char name[32];
    commandHandler handler;
} command;

static const command commands[] = {
    {"ping", &PingHandler},
    {"echo", &EchoHandler},
    {"jumpToTime", &JumpToTimeHandler},
    {"currentTime", &CurrentTimeHandler},
    {"sendFrame", &SendFrameHandler},
    {"getFramesCount", &GetFramesCountHandler},
    {"receiveFrame", &ReceiveFrameHandler},
};

static QueueHandle_t terminalQueue;

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

void TerminalSendNewLine(void)
{
    leuartPuts("\n");
}

static void terminalSendPrefix(void)
{
    leuartPuts(">");
}

void TerminalPrintf(const char* text, ...)
{
    va_list args;
    va_start(args, text);

    leuartvPrintf(text, args);

    va_end(args);
}

void TerminalPuts(const char* text)
{
    leuartPuts(text);
}

static void terminalHandleCommand(char* buffer)
{
    char* commandName;
    uint16_t argc = 0;
    char* args[8] = {0};

    parseCommandLine(buffer, &commandName, args, &argc, COUNT_OF(args));

    for (size_t i = 0; i < COUNT_OF(commands); i++)
    {
        if (strcmp(commandName, commands[i].name) == 0)
        {
            commands[i].handler(argc, args);
            TerminalSendNewLine();
        }
    }

    terminalSendPrefix();
}

static void handleIncomingChar(void* args)
{
    UNREFERENCED_PARAMETER(args);

    char input_buffer[32] = {0};
    uint32_t input_buffer_position = 0;

    while (1)
    {
        uint8_t data = 0;

        xQueueReceive(terminalQueue, &data, portMAX_DELAY);
        LOGF(LOG_LEVEL_TRACE, "Char: %c", data);
        if (data == '\n')
        {
            input_buffer[input_buffer_position] = 0;
            input_buffer_position = 0;

            terminalHandleCommand(input_buffer);
        }
        else if (input_buffer_position < sizeof(input_buffer) - 1)
        {
            input_buffer[input_buffer_position++] = data;
        }
    }
}

void TerminalInit(void)
{
    terminalQueue = xQueueCreate(32, sizeof(uint8_t));

    if (terminalQueue == NULL)
    {
        LOG(LOG_LEVEL_ERROR, "Error. Cannot create terminalIn thread.");
        return;
    }

    if (xTaskCreate(handleIncomingChar, "terminalIn", 2500, NULL, 4, NULL) != pdPASS)
    {
        LOG(LOG_LEVEL_ERROR, "Error. Cannot create terminalQueue.");
        return;
    }

    leuartInit(terminalQueue);

    terminalSendPrefix();
}
