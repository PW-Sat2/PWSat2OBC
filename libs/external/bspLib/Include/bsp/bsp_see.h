/*
 * bsp_see.h
 *
 *  Created on: 25 Apr 2014
 *      Author: pjbotma
 */
#ifndef BSP_SEE_H_
#define BSP_SEE_H_
#include "stdint.h"
#include "em_acmp.h"
#include "em_gpio.h"
#include "bsp_ebi.h"

typedef struct {
	uint8_t singleErrors;
	uint8_t doubleErrors;
	uint8_t multiErrors;
}BSP_SEE_DataScrubResult_Typedef;

/***************************************************************************//**
 * @brief
 *   SRAM selector type definition.
 * @details
 *   Select between the SRAM modules to be used in the toggle functions \link
 *   BSP_EBI_setPower \endlink and \link BSP_EBI_setBuffer \endlink.
 ******************************************************************************/
typedef enum
{
  bspSeeSram1  = 0, ///< Select SRAM module 1.
  bspSeeSram2  = 1  ///< Select SRAM module 2.
} BSP_See_SRAMSelect_TypeDef;

void setSRAM1Latched(uint8_t state);
void setSRAM2Latched(uint8_t state);
void    BSP_SEE_Init(uint32_t pieceSize, uint8_t retryCount);
void    BSP_SEE_checkMemory(void);
uint8_t BSP_SEE_isSramLatched(BSP_See_SRAMSelect_TypeDef sram);
uint8_t BSP_SEE_tryLatchupRecovery(BSP_See_SRAMSelect_TypeDef sram, uint8_t retryMax);
void    BSP_SEE_doPiecewiseDataScrub(void);
uint8_t BSP_SEE_getDataScrubCount(void);
BSP_SEE_DataScrubResult_Typedef BSP_SEE_getDataScrubResult(void);
#endif /* BSP_SEE_H_ */
