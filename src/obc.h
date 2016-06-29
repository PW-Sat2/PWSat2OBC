#ifndef OBC_H
#define OBC_H

#include <stdatomic.h>
#include "base/os.h"
#include "comm/comm.h"

/**
 * @brief Object that describes global OBS state.
 */
typedef struct
{
    /** @brief Comm driver object. */
    CommObject comm;
    /** @brief Handle to OBC initialization task. */
    OSTaskHandle initTask;
    /** @brief Flag indicating that OBC software has finished initialization process. */
    atomic_bool initialized;
} OBC;

/** @brief Global OBC object. */
extern OBC Main;

#endif
