#include "SwoEndpoint.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "swo/swo.h"
#include "system.h"

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
    void* context, bool withinISR, const char* messageHeader, const char* messageFormat, va_list messageArguments)
{
    UNREFERENCED_PARAMETER(context);
    UNREFERENCED_PARAMETER(withinISR);
    // Do not use channel 0 for logging. Leave it for other purposes.
    const uint8_t channel = 1;
    SwoPutsOnChannel(channel, messageHeader);
    SwoVPrintfOnChannel(channel, messageFormat, messageArguments);
    SwoPutsOnChannel(channel, "\n");
}

static bool Lock(void* context, bool withinISR)
{
    if (withinISR)
    {
        return (xSemaphoreTakeFromISR(context, NULL) == pdTRUE);
    }
    else
    {
        const TickType_t waitTimeout = 100 / portTICK_PERIOD_MS; // wait at most 100 ms
        return (xSemaphoreTake(context, waitTimeout) == pdTRUE);
    }
}

static bool Unlock(void* context, bool withinISR)
{
    if (withinISR)
    {
        return (xSemaphoreGiveFromISR(context, NULL) == pdTRUE);
    }
    else
    {
        return (xSemaphoreGive(context) == pdTRUE);
    }
}

static void SwoEndpointLoggerSynchronized(
    void* context, bool withinISR, const char* messageHeader, const char* messageFormat, va_list messageArguments)
{
    if (!Lock(context, withinISR))
    {
        SwoPuts("Unable to acquire SwoEndpoint semaphore.\n");
        return;
    }

    SwoEndpointLogger(context, withinISR, messageHeader, messageFormat, messageArguments);

    if (!Unlock(context, withinISR))
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
