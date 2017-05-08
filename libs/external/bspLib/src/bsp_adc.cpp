/***************************************************************************//**
 * @file	bsp_adc.h
 * @brief	BSP ADC header file.
 *
 * This header file contains all the required definitions and function
 * prototypes through which to control the CubeComputer's ADC module.
 * @author	Pieter J. Botma
 * @date	15/05/2012
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2021 ESL , http://http://www.esl.sun.ac.za/</b>
 *******************************************************************************
 *
 * This source code is the property of the ESL. The source and compiled code may
 * only be used on the CubeComputer.
 *
 * This copyright notice may not be removed from the source code nor changed.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: ESL has no obligation to
 * support this Software. ESL is providing the Software "AS IS", with no express
 * or implied warranties of any kind, including, but not limited to, any implied
 * warranties of merchantability or fitness for any particular purpose or
 * warranties against infringement of any proprietary rights of a third party.
 *
 * ESL will not be liable for any consequential, incidental, or special damages,
 * or any other relief, or for any claim by any third party, arising from your
 * use of this Software.
 *
 ******************************************************************************/
#include "bsp_adc.h"
/***************************************************************************//**
 * @addtogroup BSP_Library
 * @brief Board Support Package (<b>BSP</b>) Driver Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup ADC
 * @brief API for CubeComputer's ADC module.
 * @{
 ******************************************************************************/

volatile uint8_t isScanComplete,///< ADC scan complete flag (current & voltage).
		isSingleComplete; 		///< ADC single flag (temperature).
uint16_t adcData [CHANNELCOUNT];///< ADC channel data buffer.

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

float adc0_calTemp0; ///< On-chip calibration temperature
float adc0_temp0Read1V25; ///< On-chip calibration reading

void scanComplete (unsigned int channel, bool primary, void *user)
{
#if defined(CubeCompV2B)
	adcData [CURRENT_1V5] = (uint16_t)((adcData [CURRENT_1V5] * 1.25*1000.0) / 10 / 65536.0); // adc*milli-ref/V-to-A/resolution
	adcData [CURRENT_3V3] = (uint16_t)((adcData [CURRENT_3V3] * 1.25*1000.0) / 10 / 65536.0); // adc*milli-ref/V-to-A/resolution
	adcData [VOLTAGE_1V5] = (uint16_t)((adcData [VOLTAGE_1V5] * 1.25*1000.0) / 65536.0 / 2.0 * 3.0); // adc*milli-ref/resolution/ratio
	adcData [VOLTAGE_3V3] = (uint16_t)((adcData [VOLTAGE_3V3] * 1.25*1000.0) / 65536.0 * 3.0); // adc*milli-ref/resolution/ratio
#elif defined(CubeCompV3) || defined(CubeCompV3B)
	adcData [CHANNEL0] = (uint16_t)((adcData [CHANNEL0] * 1.25*1000.0) / 65536.0); // adc*milli-ref/V-to-A/resolution
	adcData [CHANNEL1] = (uint16_t)((adcData [CHANNEL1] * 1.25*1000.0) / 65536.0); // adc*milli-ref/V-to-A/resolution
	adcData [CHANNEL2] = (uint16_t)((adcData [CHANNEL2] * 1.25*1000.0) / 65536.0); // adc*milli-ref/resolution/ratio
	adcData [CHANNEL3] = (uint16_t)((adcData [CHANNEL3] * 1.25*1000.0) / 65536.0); // adc*milli-ref/resolution/ratio
#endif

	isScanComplete = 1;
}

void singleComplete (unsigned int channel, bool primary, void *user)
{
	// see reference manual section 28.3.4.2, equation 28.2
	float temp = adc0_calTemp0 - ( ( adc0_temp0Read1V25 - adcData [TEMPERATURE] )/4096.0*1.25*1000.0/-1.92 );

	adcData [TEMPERATURE] = (uint16_t)(temp*256.0); // assume temperature < +/-128

	isSingleComplete = 1;
}

/** @endcond */

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   21/05/2012
 *
 * This function initialises the CubeComputer's internal ADC module which is
 * used to sample the predefined channels.
 *
 ******************************************************************************/
void BSP_ADC_Init(void)
{
  // Enable clocks used by ADC Sensors
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_ADC0, true);
  CMU_ClockEnable(cmuClock_DMA, true);

  // Configure DMA for ADC Sensors
  DMA_CfgChannel_TypeDef scanChnlCfg;
  DMA_CfgChannel_TypeDef singleChnlCfg;
  DMA_CfgDescr_TypeDef   scanDescrCfg;
  DMA_CfgDescr_TypeDef   singleDescrCfg;

  // Setting call-back functions
  cb[DMA_CHANNEL_ADC_SCAN].cbFunc  = scanComplete;
  cb[DMA_CHANNEL_ADC_SCAN].userPtr = NULL;

  cb[DMA_CHANNEL_ADC_SNGL].cbFunc  = singleComplete;
  cb[DMA_CHANNEL_ADC_SNGL].userPtr = NULL;

  // Configure DMA channels used

  // Scan
  scanChnlCfg.highPri   = false;
  scanChnlCfg.enableInt = true;
  scanChnlCfg.select    = DMAREQ_ADC0_SCAN;
  scanChnlCfg.cb        = &(cb[DMA_CHANNEL_ADC_SCAN]);
  DMA_CfgChannel(DMA_CHANNEL_ADC_SCAN, &scanChnlCfg);

  // Single
  singleChnlCfg.highPri   = false;
  singleChnlCfg.enableInt = true;
  singleChnlCfg.select    = DMAREQ_ADC0_SINGLE;
  singleChnlCfg.cb        = &(cb[DMA_CHANNEL_ADC_SNGL]);
  DMA_CfgChannel(DMA_CHANNEL_ADC_SNGL, &singleChnlCfg);

  // Configure descriptors for DMA transfers

  // Single
  scanDescrCfg.dstInc  = dmaDataInc2;
  scanDescrCfg.srcInc  = dmaDataIncNone;
  scanDescrCfg.size    = dmaDataSize2;
  scanDescrCfg.arbRate = dmaArbitrate1;
  scanDescrCfg.hprot   = 0;
  DMA_CfgDescr(DMA_CHANNEL_ADC_SCAN, true, &scanDescrCfg);

  // Scan
  singleDescrCfg.dstInc  = dmaDataIncNone;
  singleDescrCfg.srcInc  = dmaDataIncNone;
  singleDescrCfg.size    = dmaDataSize2;
  singleDescrCfg.arbRate = dmaArbitrate1;
  singleDescrCfg.hprot   = 0;
  DMA_CfgDescr(DMA_CHANNEL_ADC_SNGL, true, &singleDescrCfg);

  // Configure ADC Sensors
  ADC_Init_TypeDef       init       = ADC_INIT_DEFAULT;
  ADC_InitScan_TypeDef   scanInit   = ADC_INITSCAN_DEFAULT;
  ADC_InitSingle_TypeDef singleInit = ADC_INITSINGLE_DEFAULT;

  // Init general ADC settings
  init.ovsRateSel = adcOvsRateSel16;
  init.timebase   = ADC_TimebaseCalc(0);
  init.prescale   = ADC_PrescaleCalc(7000000, 0);
  init.tailgate   = true;
  ADC_Init(ADC0, &init);

  // Init ADC scan settings
  scanInit.resolution = adcResOVS;
  scanInit.reference  = adcRef1V25;
#if defined(CubeCompV2B) || defined(CubeCompV3B)
  scanInit.input      = ADC_SCANCTRL_INPUTMASK_CH4 |
                        ADC_SCANCTRL_INPUTMASK_CH5 |
                        ADC_SCANCTRL_INPUTMASK_CH6 |
                        ADC_SCANCTRL_INPUTMASK_CH7;
#elif defined(CubeCompV3)

  scanInit.input      = ADC_SCANCTRL_INPUTMASK_CH0 |
                        ADC_SCANCTRL_INPUTMASK_CH1 |
                        ADC_SCANCTRL_INPUTMASK_CH4 |
                        ADC_SCANCTRL_INPUTMASK_CH5;
#endif
  ADC_InitScan(ADC0, &scanInit);

  // Init ADC single settings (temp cannot be selected as scan input)
  singleInit.reference  = adcRef1V25;
  singleInit.acqTime    = adcAcqTime32;
  singleInit.input      = adcSingleInpTemp;
  ADC_InitSingle(ADC0, &singleInit);

  // Calibration values
  adc0_calTemp0      = (float)((DEVINFO->CAL & _DEVINFO_CAL_TEMP_MASK) >> _DEVINFO_CAL_TEMP_SHIFT);
  adc0_temp0Read1V25 = (float)((DEVINFO->ADC0CAL2 & _DEVINFO_ADC0CAL2_TEMP1V25_MASK) >> _DEVINFO_ADC0CAL2_TEMP1V25_SHIFT);
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   21/05/2012
 *
 * This function updates the ADC data buffer by scanning the ADC channels
 * (current and voltage levels) and sampling the on-chip temperature sensor.
 *
 * @param [in] wait
 *   Determines if the function should wait, in sleep mode, until update
 *   completes or immediately return from subroutine.
 *
 ******************************************************************************/
void BSP_ADC_update (uint8_t wait)
{
  DMA_ActivateBasic(DMA_CHANNEL_ADC_SCAN, true, false, adcData,
                     (void *)((uint32_t) &(ADC0->SCANDATA)), 4-1);

  DMA_ActivateBasic(DMA_CHANNEL_ADC_SNGL, true, false, &(adcData[TEMPERATURE]),
                       (void *)((uint32_t) &(ADC0->SINGLEDATA)), 1-1);

  isScanComplete   = 0;
  isSingleComplete = 0;

  ADC_Start(ADC0, adcStartScanAndSingle);

  // wait for temp measurement to finnish
  while (wait && !BSP_ADC_isUpdateComplete())
  {
	  EMU_EnterEM1();
  }
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   14/03/2014
 *
 * This function updates the ADC temperature value by sampling the on-chip
 * temperature sensor.
 *
 * @param [in] wait
 *   Determines if the function should wait, in sleep mode, until update
 *   completes or immediately return from subroutine.
 *
 ******************************************************************************/
void BSP_ADC_updateTemp (void)
{
	ADC0->CTRL &= ~ADC_CTRL_TAILGATE; // refer EFM ref. man. 28.3.7.3
	ADC_Start(ADC0, adcStartSingle);

	while ( !(ADC_IntGet(ADC0) & ADC_IF_SINGLE) )
	{
		EMU_EnterEM1();
	}

	ADC_IntClear(ADC0, ADC_IFC_SINGLE);
	ADC0->CTRL |= ADC_CTRL_TAILGATE;

	// see reference manual section 28.3.4.2, equation 28.2
	float temp = adc0_calTemp0 - ( ( adc0_temp0Read1V25 - ((float)ADC_DataSingleGet(ADC0)) )/4096.0*1.25*1000.0/-1.92 );

	adcData [TEMPERATURE] = (uint16_t)(temp*256.0); // assume temperature < +/-128
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   21/05/2012
 *
 * This function return true if both the channels have been scanned and the
 * temperature has been sampled.
 *
 * @return
 * 	 True if both the channels have been scanned and the.
 ******************************************************************************/
uint8_t BSP_ADC_isUpdateComplete (void)
{
  return (isScanComplete && isSingleComplete);
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   21/05/2012
 *
 * This function returns the value, as an unsigned integer, of a specified
 * ADC \b channel.
 *
 * @param [in] channel
 *   The specified channel which value should be returned.
 * @return
 * 	 The value of the specified ADC channel.
 ******************************************************************************/
uint16_t BSP_ADC_getData (ADC_Channel_TypeDef channel)
{
	switch (channel)
	{
	case (0): 	return adcData [0];
	case (1): 	return adcData [1];
	case (2): 	return adcData [2];
	case (3): 	return adcData [3];
	case (TEMPERATURE): return adcData [TEMPERATURE];
	default: 			return 0;
	}
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   21/05/2012
 *
 * This function returns the pointer to the data buffer containing the
 * latest values of all the ADC channels.
 *
 * @return
 * 	 The pointer to the ADC data buffer.
 ******************************************************************************/
uint16_t* BSP_ADC_getDataBuff (void)
{
  return adcData;
}

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

void ADC0_IRQHandler(void)
{
  // Reset all pending interupts
  ADC_IntClear(ADC0, ADC_IFC_SINGLE);

  float tmp = adc0_calTemp0 + ( ( (ADC0->SINGLEDATA) - adc0_temp0Read1V25 )/4096*1.25*1000/-1.92 );

  adcData [TEMPERATURE] = (uint16_t) (tmp*256.0);

  isSingleComplete = 1;
}

/** @endcond */

/** @} (end addtogroup ADC) */
/** @} (end addtogroup BSP_Library) */
