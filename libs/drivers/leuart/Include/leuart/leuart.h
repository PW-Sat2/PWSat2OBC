#ifndef SRC_LEUART_H_
#define SRC_LEUART_H_

#include <stdarg.h>
#include "line_io.h"

/**
 * @defgroup leuart LEUART driver
 *
 * @{
 */

/**
 * @brief Initializes LEUART driver
 * @param[out] io Line IO interface
 */
void LeuartLineIOInit(LineIO* io);

/** @} */

#endif
