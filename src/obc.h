#ifndef OBC_H
#define OBC_H

#include <stdatomic.h>
#include "base/os.h"
#include "comm/comm.h"
#include "fs/fs.h"

/**
 * @brief Object that describes global OBS state.
 */
typedef struct
{
    /** @brief Comm driver object. */
    CommObject comm;
    FileSystem fs;
    /** @brief Handle to OBC initialization task. */
    OSTaskHandle initTask;
    /** @brief Flag indicating that OBC software has finished initialization process. */
    atomic_bool initialized;
} OBC;

/** @brief Global OBC object. */
extern OBC Main;

#endif
