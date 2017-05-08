/***************************************************************************/ /**
  * @file
  * @brief	CubeComputer Comms Definitions.
  *
  * This file contains the definitions and functions to react to specified
  * telemetry and telcommand.
  *
  * @author	Pieter J. Botma
  * @date	02/04/2013
  *
  ******************************************************************************/

#ifndef __COMMS_H
#define __COMMS_H

#include "includes.h"

void COMMS_Init(void);
void COMMS_processMsg(void);

extern uint8_t uartReceived;

#endif
