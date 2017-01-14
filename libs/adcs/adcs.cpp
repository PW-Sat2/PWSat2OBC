#include "adcs.h"
#include "logger/logger.h"
#include "system.h"

using namespace std::chrono_literals;

static void HandleCommand(ADCSContext* context, ADCSCommand command)
{
    switch (command)
    {
        case ADCSCommandTurnOff:
            context->CurrentMode = ADCSModeNone;
            break;
        case ADCSCommandDetumble:
            context->CurrentMode = ADCSModeDetumbling;
            break;
        case ADCSCommandSunPoint:
            context->CurrentMode = ADCSModeSunPointing;
            break;
    }
}

static void ADCSTask(void* arg)
{
    ADCSContext* context = (ADCSContext*)arg;

    UNREFERENCED_PARAMETER(context);

    while (1)
    {
        ADCSCommand command;

        if (System::QueueReceive(context->CommandQueue, &command, 0ms))
        {
            LOGF(LOG_LEVEL_INFO, "[ADCS]Received command %d", command);

            HandleCommand(context, command);
        }

        LOGF(LOG_LEVEL_TRACE, "[ADCS]Running ADCS loop. Mode: %d", context->CurrentMode);
        System::SleepTask(5s);
    }
}

static void ADCSSendCommand(ADCSContext* context, ADCSCommand command)
{
    LOGF(LOG_LEVEL_INFO, "[ADCS]Commanding to %d", command);
    System::QueueOverwrite(context->CommandQueue, &command);
}

void InitializeADCS(ADCSContext* context)
{
    context->Command = ADCSSendCommand;
    context->CommandQueue = System::CreateQueue(1, sizeof(ADCSCommand));
    System::CreateTask(ADCSTask, "ADCSTask", 1024, context, TaskPriority::P4, &context->Task);
}
