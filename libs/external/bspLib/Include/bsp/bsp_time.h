/*
 * bsp_time.h
 *
 *  Created on: 11 Sep 2015
 *      Author: cjgroenewald
 */
#ifndef BSP_TIME_H_
#define BSP_TIME_H_

#include "em_cmu.h"

volatile uint32_t msTicks; 					///< Runtime counter in ms, used for general purpose.

void BSP_Clocks_Init(void);
void SysTick_Handler(void);
void Delay(uint32_t delay);
#endif /* BSP_TIME_H_ */
