/***************************************************************************//**
 * @file
 * @brief	CubeComputer Test Program Test Definitions.
 *
 * This file contains the definitions for functions written to test the
 * functionality of all the peripherals and subsystems of CubeComputer.
 *
 * @author	Pieter J. Botma
 * @date	27/02/2013
 *
 ******************************************************************************/

#ifndef __TEST_H
#define __TEST_H

#include "includes.h"

extern uint8_t testResultEBI[3];
extern uint8_t testResultMicroSD;

void TEST_RTC(void);
void TEST_EBI(void);
void TEST_I2C(void);
void TEST_ADC(void);
void TEST_microSD(void);
void TEST_SRAM(void);
void TEST_Wdg(bool Internal, bool External);
void TEST_I2CDMA();
#endif
