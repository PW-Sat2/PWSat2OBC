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

static const command commands[] = {
    {"ping", PingHandler},
    {"echo", EchoHandler},
    {"jumpToTime", JumpToTimeHandler},
    {"currentTime", CurrentTimeHandler},
    {"takeRAWPhoto", &TakeRAWPhotoHandler},
    {"takeJPEGPhoto", &TakeJPEGPhotoHandler},
    {"sendFrame", SendFrameHandler},
    {"getFramesCount", GetFramesCountHandler},
    {"receiveFrame", ReceiveFrameHandler},
    {"pauseComm", CommandPauseComm},
    {"getState", OBCGetState},
    {"listFiles", FSListFiles},
    {"writeFile", FSWriteFile},
    {"readFile", FSReadFile},
    {"stateCmd", CommandByTerminal} //
};

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
    Main.IO.Puts(&Main.IO, "\n");
}

static void terminalSendPrefix(void)
{
    Main.IO.Puts(&Main.IO, ">");
}

void TerminalPrintf(const char* text, ...)
{
    va_list args;
    va_start(args, text);

    Main.IO.VPrintf(&Main.IO, text, args);

    va_end(args);
}

void TerminalPuts(const char* text)
{
    Main.IO.Puts(&Main.IO, text);
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

    bool firstRun = true;

    while (1)
    {
        char input_buffer[100] = {0};

        if (!firstRun)
        {
            terminalSendPrefix();
        }

        firstRun = false;

        Main.IO.Readline(&Main.IO, input_buffer, COUNT_OF(input_buffer));

        LOGF(LOG_LEVEL_INFO, "Received line %s", input_buffer);

        terminalHandleCommand(input_buffer);
    }
}

void TerminalInit(void)
{
    if (xTaskCreate(handleIncomingChar, "terminalIn", 2500, NULL, 4, NULL) != pdPASS)
    {
        LOG(LOG_LEVEL_ERROR, "Error. Cannot create terminalQueue.");
        return;
    }

    Main.IO.Puts(&Main.IO, "@");
}
