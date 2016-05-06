#include "SwoEndpoint.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "swo/swo.h"
#include "system.h"
#include <assert.h>
#include <stdint.h>

void* SwoEndpointInit(void)
{
    SemaphoreHandle_t handle = xSemaphoreCreateBinary();
    if (handle != NULL)
    {
        if (xSemaphoreGive(handle) != pdTRUE)
        {
            vSemaphoreDelete(handle);
            handle = NULL;
            SwoPuts("Unable to initialize SwoEndpoint semaphore.\n");
        }
    }
    else
    {
        SwoPuts("Unable to create SwoEndpoint state.\n");
    }

    return handle;
}

void SwoEndpointClose(void* handle)
{
    if (handle != NULL)
    {
        vSemaphoreDelete(handle);
    }
}

static void SwoEndpointLogger(
    void* context, const char* messageHeader, const char* messageFormat, va_list messageArguments)
{
    UNREFERENCED_PARAMETER(context);
    // Do not use channel 0 for logging. Leave it for other purposes.
    const uint8_t channel = 1;
    SwoPutsOnChannel(channel, messageHeader);
    SwoVPrintfOnChannel(channel, messageFormat, messageArguments);
    SwoPutsOnChannel(channel, "\n");
}

static void SwoEndpointLoggerSynchronized(
    void* context, const char* messageHeader, const char* messageFormat, va_list messageArguments)
{
    const TickType_t waitTimeout = 100 / portTICK_PERIOD_MS; // wait at most 100 ms
    if (xSemaphoreTake(context, waitTimeout) != pdTRUE)
    {
        SwoPuts("Unable to acquire SwoEndpoint semaphore.\n");
        return;
    }

    SwoEndpointLogger(context, messageHeader, messageFormat, messageArguments);

    if (xSemaphoreGive(context) != pdTRUE)
    {
        SwoPuts("Unable to release SwoEndpoint semaphore.\n");
    }
}

LoggerProcedure SwoGetEndpoint(void* context)
{
    if (context != NULL)
    {
        return &SwoEndpointLoggerSynchronized;
    }
    else
    {
        return &SwoEndpointLogger;
    }
}
