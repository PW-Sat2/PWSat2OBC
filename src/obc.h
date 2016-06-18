#ifndef OBC_H
#define OBC_H

#include <stdatomic.h>
#include "base/os.h"
#include "comm/comm.h"

typedef struct
{
    CommObject comm;
    OSTaskHandle initTask;
    atomic_bool initialized;
} OBC;

extern OBC Main;

#endif
