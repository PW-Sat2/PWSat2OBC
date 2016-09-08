#ifndef LIBS_SWO_ENDPOINT_H
#define LIBS_SWO_ENDPOINT_H

#pragma once

#include "logger/logger.h"

/**
 * @defgroup SwoEndpoint SWO Endpoint for Logger
 * @ingroup Logger
 *
 * @brief Logger data sink that forwards the logged entries to the Serial Wire Output (SWO) channel 1.
 * @{
 */

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
 * @brief Returns swo endpoint entry point appropriate for the passed swo endpoint.
 * @param[in] handle SwoEndpoint context whose entry point should be returned.
 * @return Swo Endpoint entry point.
 */
LoggerProcedure SwoGetEndpoint(void* handle);

/** @}*/

#endif
