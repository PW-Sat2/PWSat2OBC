/*
 * bsp_see.c
 *
 *  Created on: 25 Apr 2014
 *      Author: pjbotma
 */

#include "bsp_see.h"
uint8_t latchupRetryCount = 0;

volatile static uint8_t sram1Latched = 0;
volatile static uint8_t sram2Latched = 0;

uint8_t  dataScrubCount = 0;
uint32_t dataScrubOffset = 0;
uint32_t dataScrubPieceSize = BSP_EBI_SRAM_SIZE;
BSP_SEE_DataScrubResult_Typedef dataScrubCurResult;
BSP_SEE_DataScrubResult_Typedef dataScrubPrevResult;

void checkAndLogErrors(void);

void setSRAM1Latched(uint8_t state)
{
	sram1Latched = state;
}

void setSRAM2Latched(uint8_t state)
{
	sram2Latched = state;
}
/**************************************************************************//**
 * @brief RTC_IRQHandler
 * Interrupt Service Routine for analog comparators ACMP0 and ACMP1
 *****************************************************************************/
void ACMP0_IRQHandler(void)
{
	if(ACMP0->IF & ACMP_IF_EDGE)
	{
		#if defined (CubeCompV3) || defined(CubeCompV3B)
			BSP_EBI_Sram1FPGAOff();
		#endif
		BSP_EBI_Sram1BuffOn();
		BSP_EBI_Sram1PowOff();

		sram1Latched = 1;

		ACMP_IntClear(ACMP0, ACMP_IFC_EDGE);
	}

	if(ACMP1->IF & ACMP_IF_EDGE)
	{
		#if defined (CubeCompV3) || defined(CubeCompV3B)
			BSP_EBI_Sram2FPGAOff();
		#endif
		BSP_EBI_Sram2BuffOn();
		BSP_EBI_Sram2PowOff();

		sram2Latched = 1;

		ACMP_IntClear(ACMP1, ACMP_IFC_EDGE);
	}
}

void BSP_SEE_Init(uint32_t pieceSize, uint8_t retryCount)
{
	sram1Latched = 0;
	sram2Latched = 0;
	latchupRetryCount = retryCount;

	dataScrubCount = 0;
	dataScrubOffset = 0;
	dataScrubPieceSize = pieceSize;
	dataScrubCurResult.singleErrors = 0;
	dataScrubCurResult.doubleErrors = 0;
	dataScrubCurResult.multiErrors  = 0;
	dataScrubPrevResult.singleErrors = 0;
	dataScrubPrevResult.doubleErrors = 0;
	dataScrubPrevResult.multiErrors  = 0;
}

void BSP_SEE_checkMemory(void)
{
	 BSP_SEE_doPiecewiseDataScrub();

	 if(sram1Latched)
		 BSP_SEE_tryLatchupRecovery(bspEbiSram1,latchupRetryCount);

	 if(sram2Latched)
		 BSP_SEE_tryLatchupRecovery(bspEbiSram2,latchupRetryCount);
}

uint8_t BSP_SEE_isSramLatched(BSP_See_SRAMSelect_TypeDef sram)
{
	switch(sram)
	{
	case bspEbiSram1:
		return sram1Latched;
	case bspEbiSram2:
		return sram2Latched;
	default:
		return 0;
	}
}

/***************************************************************************//**
 * @brief
 *   Tries to recover a SRAM from a latched state.
 * @detail
 *   The latchup recovery process consists of power cycling the SRAM module a
 *   specified amount of time.
 *
 * @param
 *
 ******************************************************************************/
uint8_t BSP_SEE_tryLatchupRecovery(BSP_See_SRAMSelect_TypeDef sram, uint8_t retryMax)
{
	uint8_t recovered, retryCount;

	recovered = 0;

	// try to recover latch
	for(retryCount = 0; (retryCount < retryMax) && !recovered; retryCount++)
	{
		recovered = BSP_EBI_enableSRAM(sram);
	}

	// reset latchup flags
	switch(sram)
	{
	case bspEbiSram1:
		sram1Latched = (recovered) ? 0 : 1;
		break;
	case bspEbiSram2:
		sram2Latched = (recovered) ? 0 : 1;
		break;
	}

	return recovered;
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   25/04/2014
 *
 * This function checks a piece of the external SRAM memory for single event
 * upsets. The size of the piece being checked was defined during initialization.
 *
 * @note
 *   Data-scrubbing routine consists of reading and writing back all the data
 *   in external SRAM. This is needed because the EDAC only corrects data when
 *   it is read. The correctly read data has to be re-written to SRAM in order
 *   to correct the corrupt data on the SRAM module. Running the data-scrubbing
 *   routine once in a while will prevent the build up of errors.
 *
 ******************************************************************************/
void BSP_SEE_doPiecewiseDataScrub(void)
{
	uint8_t *curAddress, *startAddress, *endAddress, tempData;

	startAddress = (uint8_t*)(BSP_EBI_SRAM1_BASE + dataScrubOffset);
	endAddress   = startAddress + dataScrubPieceSize;

	// trim data scrub piece size if overflow will occur
	if(endAddress > (uint8_t*)(BSP_EBI_SRAM1_BASE+BSP_EBI_SRAM_SIZE))
	{
		endAddress =  (uint8_t*)(BSP_EBI_SRAM1_BASE+BSP_EBI_SRAM_SIZE);
	}

	for(curAddress = startAddress; curAddress < endAddress; curAddress++)
	{
		tempData = *curAddress; // EDAC corrects read data
		*curAddress = tempData; // save correct data to SRAM

		checkAndLogErrors();
	}

	dataScrubOffset += dataScrubPieceSize;

	if(dataScrubOffset >= BSP_EBI_SRAM_SIZE)
	{
		dataScrubCount++;
		dataScrubOffset = 0;

		dataScrubPrevResult = dataScrubCurResult;

		dataScrubCurResult.singleErrors = 0;
		dataScrubCurResult.doubleErrors = 0;
		dataScrubCurResult.multiErrors = 0;
	}
}

void checkAndLogErrors(void)
{
	uint8_t errors;

	errors = GPIO_IntGet() & BSP_EBI_EDAC_ERROR_PINS;

	switch(errors)
	{
	case BSP_EBI_EDAC_ERROR_SINGLE:
		dataScrubCurResult.singleErrors++;
		break;

	case BSP_EBI_EDAC_ERROR_DOUBLE:
		dataScrubCurResult.doubleErrors++;
		break;

	case BSP_EBI_EDAC_ERROR_MULTI:
		dataScrubCurResult.multiErrors++;
		break;

	default:
		break;
	}

	GPIO_IntClear(BSP_EBI_EDAC_ERROR_PINS);
}

uint8_t BSP_SEE_getDataScrubCount(void)
{
	return dataScrubCount;
}

BSP_SEE_DataScrubResult_Typedef BSP_SEE_getDataScrubResult(void)
{
	return dataScrubPrevResult;
}
