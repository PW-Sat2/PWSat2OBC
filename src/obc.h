#ifndef OBC_H
#define OBC_H

#include "base/os.h"
#include "comm/comm.h"

typedef struct
{
    CommObject comm;
    OSTaskHandle initTask;
    bool initialized;
} OBC;

extern OBC Main;

#endif
