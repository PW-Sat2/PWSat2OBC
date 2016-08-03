#ifndef SRC_OPENSAIL_H_
#define SRC_OPENSAIL_H_

#include "time/TimePoint.h"
#include "base/os.h"

typedef struct
{
    OSTaskHandle SailTaskHandle;
    OSSemaphoreHandle SemaphoreHandle;
} OpenSailContext;

bool OpenSailInit(OpenSailContext* context);

void OpenSailTimeHandler(void* context, TimePoint currentTime);

#endif /* SRC_OPENSAIL_H_ */
