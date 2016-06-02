#ifndef LIBS_SWO_ENDPOINT_H
#define LIBS_SWO_ENDPOINT_H

#pragma once

#include "logger/logger.h"

/**
 * @brief Initializes swo logger endpoint.
 *
 * @returns The swo endpoint handle.
 */
void* SwoEndpointInit(void);

/**
 * @brief Shuts down swo logger endpoint associated with by passed context.
 * @param[in] handle SwoEndpoint context that should be destroyed.
 *  This parameter can be NULL.
 */
void SwoEndpointClose(void* handle);

/**
 * @brief Returns swo endpoint entry point apropriate for the
 * passed swo endpoint.
 * @return Swo Endpoint entry point.
 */
LoggerProcedure SwoGetEndpoint(void* handle);

#endif
