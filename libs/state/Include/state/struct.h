#ifndef LIBS_STATE_INCLUDE_STATE_STRUCT_H_
#define LIBS_STATE_INCLUDE_STATE_STRUCT_H_

#include <stdbool.h>
#include <stdint.h>
#include "adcs/adcs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TerminalCommandNone = 0,
    TerminalCommandADCSTurnOff = 1 << 0,
    TerminalCommandADCSDetumble = 1 << 1,
    TerminalCommandADCSSunPoint = 1 << 2
} TerminalCommand;

typedef struct
{
    bool Flag;
    int32_t NumValue;
    TerminalCommand RequestedCommand;
    uint32_t Time;
    bool AntennaDeployed;
    bool SailOpened;
    struct
    {
        ADCSMode CurrentMode;
    } ADCS;
} SystemState;

#ifdef __cplusplus
}
#endif

#endif /* LIBS_STATE_INCLUDE_STATE_STRUCT_H_ */
