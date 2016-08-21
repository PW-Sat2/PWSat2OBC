#ifndef LIBS_MISSION_INCLUDE_MISSION_ADCS_H_
#define LIBS_MISSION_INCLUDE_MISSION_ADCS_H_

#include "base/os.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { ADCSModeNone, ADCSModeDetumbling, ADCSModeSunPointing } ADCSMode;

typedef enum { ADCSCommandTurnOff, ADCSCommandDetumble, ADCSCommandSunPoint } ADCSCommand;

typedef struct _ADCSContext
{
    OSTaskHandle Task;
    OSQueueHandle CommandQueue;
    ADCSMode CurrentMode;
    void (*Command)(struct _ADCSContext* context, ADCSCommand command);
} ADCSContext;

void InitializeADCS(ADCSContext* context);

#ifdef __cplusplus
}
#endif

#endif /* LIBS_MISSION_INCLUDE_MISSION_ADCS_H_ */
