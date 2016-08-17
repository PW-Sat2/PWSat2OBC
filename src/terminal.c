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
#include "obc.h"
#include "swo/swo.h"
#include "system.h"
#include "terminal.h"

typedef void (*commandHandler)(uint16_t argc, char* argv[]);

typedef struct
{
    char name[32];
    commandHandler handler;
} command;

static const command commands[] = {{"ping", PingHandler},
    {"echo", EchoHandler},
    {"jumpToTime", JumpToTimeHandler},
    {"currentTime", CurrentTimeHandler},
    {"sendFrame", SendFrameHandler},
    {"getFramesCount", GetFramesCountHandler},
    {"receiveFrame", ReceiveFrameHandler},
    {"pauseComm", CommandPauseComm},
    {"getState", OBCGetState},
    {"listFiles", FSListFiles},
    {"writeFile", FSWriteFile},
    {"readFile", FSReadFile}};

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
    Main.IO.Puts("\n");
}

static void terminalSendPrefix(void)
{
    Main.IO.Puts(">");
}

void TerminalPrintf(const char* text, ...)
{
    va_list args;
    va_start(args, text);

    Main.IO.VPrintf(text, args);

    va_end(args);
}

void TerminalPuts(const char* text)
{
    Main.IO.Puts(text);
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
}

static void handleIncomingChar(void* arg)
{
    UNREFERENCED_PARAMETER(arg);

    char input_buffer[100] = {0};
    uint32_t input_buffer_position = 0;

    bool firstRun = true;

    while (1)
    {
        memset(input_buffer, 0, sizeof(input_buffer));

        if (!firstRun)
        {
            terminalSendPrefix();
        }

        firstRun = false;

        Main.IO.Readline(input_buffer, COUNT_OF(input_buffer));

        LOGF(LOG_LEVEL_INFO, "Received line %s", input_buffer);

        terminalHandleCommand(input_buffer);

        //        xQueueReceive(terminalQueue, &data, portMAX_DELAY);
        //#if 0
        //        LOGF(LOG_LEVEL_INFO, "RC: 0x%x, %c", (int)data, data);
        //#endif
        //        if (data == '\n')
        //        {
        //            input_buffer[input_buffer_position] = 0;
        //            input_buffer_position = 0;
        //
        //            terminalHandleCommand(input_buffer);
        //        }
        //        else if (input_buffer_position < sizeof(input_buffer) - 1)
        //        {
        //            input_buffer[input_buffer_position++] = data;
        //        }
    }
}

void TerminalInit(LineIO* io)
{
    terminalQueue = xQueueCreate(128, sizeof(uint8_t));

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
    LeuartLineIOInit(io);

    io->Puts("@");
}
