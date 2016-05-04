#include "SwoEndpoint.h"
#include "swo/swo.h"
#include "system.h"
#include <assert.h>

static_assert(LOG_LEVEL_MAX < 30, "Too many log levels");

void SwoEndpointLogger(void* context, const char* messageHeader, const char* messageFormat, va_list messageArguments)
{
    UNREFERENCED_PARAMETER(context);
    // Do not use channel 0 for logging. Leave it for other purposes.
    const int channel = 1;
    SwoPutsOnChannel(channel, messageHeader);
    SwoVPrintfOnChannel(channel, messageFormat, messageArguments);
    SwoPutsOnChannel(channel, "\n");
}
