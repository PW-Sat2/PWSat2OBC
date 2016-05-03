#ifndef LIBS_SWO_ENDPOINT_H
#define LIBS_SWO_ENDPOINT_H

#pragma once

#include <stdarg.h>
#include "Logger/Logger.h"

void SwoEndpointLogger(void* context,
    enum LogLevel messageLevel,
    const char* messageHeader,
    const char* messageFormat,
    va_list messageArguments);

#endif
