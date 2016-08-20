#include "logger/logger.h"
#include "system.h"

#include "devices/eps.h"
#include "openSail.h"

#define SAILOPENTIME 2500000

static void openSail(void)
{
    if (!EpsOpenSail())
    {
        LOG(LOG_LEVEL_ERROR, "Failed to open sail");
    }
}

static void openSailTask(void* parameter)
{
    OpenSailContext* context = (OpenSailContext*)parameter;

    for (;;)
    {
        const OSResult result = System.TakeSemaphore(context->SemaphoreHandle, MAX_DELAY);
        if (result == OSResultSuccess)
        {
            LOG(LOG_LEVEL_INFO, "time to open sail.");

            openSail();

            System.SuspendTask(NULL);
        }
        else if (result != OSResultTimeout)
        {
            LOG(LOG_LEVEL_ERROR, "Open Sail semaphore wait failure");
        }
    }
}

bool OpenSailInit(OpenSailContext* context)
{
    context->SemaphoreHandle = System.CreateBinarySemaphore();
    if (context->SemaphoreHandle == NULL)
    {
        LOG(LOG_LEVEL_ERROR, "Unable to create openSail semaphore");
        return false;
    }

    if (System.CreateTask(openSailTask, "openSail", 1024, context, 4, &context->SailTaskHandle) != OSResultSuccess)
    {
        LOG(LOG_LEVEL_ERROR, "Unable to create openSail task");
        return false;
    }

    return true;
}

void OpenSailTimeHandler(void* context, TimePoint currentTime)
{
    const OpenSailContext* sailContext = (OpenSailContext*)context;
    const TimeSpan span = TimePointToTimeSpan(currentTime);
    if (TimeSpanLessThan(TimeSpanFromMilliseconds(SAILOPENTIME), span))
    {
        if (System.GiveSemaphore(sailContext->SemaphoreHandle) != OSResultSuccess)
        {
            LOG(LOG_LEVEL_ERROR, "Unable to signal task to open sail. ");
        }
    }
}
