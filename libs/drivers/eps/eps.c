#include "base/os.h"

#include "eps.h"

#include "i2c/i2c.h"
#include "logger/logger.h"

#include "system.h"

#define EPS_ADDRESS 12

typedef enum {
    EPS_LCL_SAIL_0 = 0,
    EPS_LCL_SAIL_1 = 1,
} EpsLcl;

static bool epsControlLCL(EpsLcl lcl, uint8_t state)
{
    uint8_t data[] = {1 + lcl, state};
    I2C_TransferReturn_TypeDef result = I2CWrite(EPS_ADDRESS, data, COUNT_OF(data));

    if (result != i2cTransferDone)
    {
        LOGF(LOG_LEVEL_ERROR, "[EPS] ControlLCL %d to state %d failed: %d", lcl, state, result);
    }

    return result == i2cTransferDone;
}

bool EpsOpenSail(void)
{
    LOG(LOG_LEVEL_INFO, "[EPS] Opening sail");

    if (!epsControlLCL(EPS_LCL_SAIL_0, true))
    {
        return false;
    }
    System.SleepTask(100);

    if (!epsControlLCL(EPS_LCL_SAIL_0, false))
    {
        return false;
    }
    System.SleepTask(100);

    if (!epsControlLCL(EPS_LCL_SAIL_1, true))
    {
        return false;
    }
    System.SleepTask(100);

    if (!epsControlLCL(EPS_LCL_SAIL_1, false))
    {
        return false;
    }
    System.SleepTask(100);

    return true;
}

void EpsInit(void)
{
}
