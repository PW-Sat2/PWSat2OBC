/***************************************************************************//**
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

#define COMMS_I2C_TYPE  0x01
#define COMMS_I2C_READ  0x01
#define COMMS_I2C_WRITE 0x00

#define COMMS_ID_TYPE 0x80
#define COMMS_ID_TLM  0x80
#define COMMS_ID_TCMD 0x00

#define COMMS_ERROR_TCMDBUFOF 	1
#define COMMS_ERROR_UARTTLM		2
#define COMMS_ERROR_I2CTLM  	3

#define COMMS_TCMDERR_PARAMUF 	1
#define COMMS_TCMDERR_PARAMOF 	2

#define COMMS_TCMD_BUFFLEN   	4
#define COMMS_TCMD_PARAMLEN  	8

void COMMS_Init(void);
void COMMS_processTCMD(void);

extern uint8_t debugStr[512];
extern uint16_t debugLen;

#endif
