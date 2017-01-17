/***************************************************************************//**
 * @file	bsp_ebi.h
 * @brief	BSP EBI header file.
 *
 * This header file contains all the required definitions and function
 * prototypes through which to control the CubeComputer's I2C channels.
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
#ifndef __BSP_EBI_H
#define __BSP_EBI_H
#include <stdbool.h>
#include "em_ebi.h"
#include "bsp_acmp.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "lld.h"
#include "bsp_time.h"
#include "bsp_see.h"
/***************************************************************************//**
 * @addtogroup BSP_Library
 * @brief Board Support Package (<b>BSP</b>) Driver Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup EBI
 * @brief API for CubeComputer's external bus interface.
 * @{
 ******************************************************************************/

#define BSP_EBI_EEPROM_BASE EBI_BankAddress(EBI_BANK0) ///< EEPROM is mapped to bank 0 on CubeComputer.
#define BSP_EBI_FLASH_BASE  EBI_BankAddress(EBI_BANK1) ///< Flash is mapped to bank 1 on CubeComputer.
#define BSP_EBI_SRAM1_BASE  EBI_BankAddress(EBI_BANK2) ///< SRAM module 1 is mapped to bank 2 on CubeComputer.
//#define BSP_EBI_SRAM2_BASE  EBI_BankAddress(EBI_BANK3) ///< SRAM module 2 is mapped to bank 3 on CubeComputer.

#define BSP_EBI_EEPROM_SIZE   (32*1024) ///< Size of external EEPROM module
#define BSP_EBI_FLASH_SIZE  (4096*1024) ///< Size of external FLASH module
#define BSP_EBI_SRAM_SIZE   (1024*1024) ///< Size of external SRAM modules

#define BSP_EBI_FLASH_SSECTOR_SIZE ( 8*1024) ///< Size of external FLASH module small sector
#define BSP_EBI_FLASH_LSECTOR_SIZE (64*1024) ///< Size of external FLASH module large sector

#define BSP_EBI_EEPROM_POLL_MASK 0x80 ///< Mask for EEPROM data to poll write sequence (see datasheet section 20)

#if defined(CubeCompV2B)
#define BSP_EBI_SRAM_POWPORT gpioPortC ///< Port location of SRAM1 power switch enable
#define BSP_EBI_SRAM1_POWPIN        12 ///< Pin location of SRAM1 power switch enable
#define BSP_EBI_SRAM2_POWPIN        13 ///< Pin location of SRAM2 power switch enable
#elif defined (CubeCompV3)
#define BSP_EBI_SRAM_POWPORT gpioPortD ///< Port location of SRAM1 power switch enable
#define BSP_EBI_SRAM1_POWPIN         7 ///< Pin location of SRAM1 power switch enable
#define BSP_EBI_SRAM2_POWPIN         8 ///< Pin location of SRAM2 power switch enable
#elif defined (CubeCompV3B)
#define BSP_EBI_SRAM_POWPORT gpioPortC ///< Port location of SRAM1 power switch enable
#define BSP_EBI_SRAM1_POWPIN         0 ///< Pin location of SRAM1 power switch enable
#define BSP_EBI_SRAM2_POWPIN         1 ///< Pin location of SRAM2 power switch enable
#endif

#define BSP_EBI_SRAM_BUFPORT gpioPortC ///< Port location of SRAM1 power switch enable
#define BSP_EBI_SRAM1_BUFPIN        14 ///< Pin location of SRAM1 power switch enable
#define BSP_EBI_SRAM2_BUFPIN        15 ///< Pin location of SRAM2 power switch enable

#define BSP_EBI_EDAC_CONTROLPORT gpioPortE ///< Port location of FPGA control pins
#define BSP_EBI_EDAC_CONTROLPIN1		 2 ///< Pin location of FPGA control pin for SRAM1
#define BSP_EBI_EDAC_CONTROLPIN2		 3 ///< Pin location of FPGA control pin for SRAM2

#define BSP_EBI_EDAC_ERROR_PORT gpioPortB
#define BSP_EBI_EDAC_ERROR_PINS    0x0005

#define BSP_EBI_EDAC_ERROR_NONE   0x0005
#define BSP_EBI_EDAC_ERROR_SINGLE 0x0004
#define BSP_EBI_EDAC_ERROR_DOUBLE 0x0001
#define BSP_EBI_EDAC_ERROR_MULTI  0x0000

#define BSP_EBI_TIMEOUT_INRUSH 0x2E0 ///< Timeout counter for SRAM in-rush-current //0xff

/**************************************************************************//**
 * @name External SRAM Function Defines
 *****************************************************************************/
/** @{ */
#define BSP_EBI_Sram1BuffOn() GPIO_PinOutClear(BSP_EBI_SRAM_BUFPORT, BSP_EBI_SRAM1_BUFPIN)
#define BSP_EBI_Sram2BuffOn() GPIO_PinOutClear(BSP_EBI_SRAM_BUFPORT, BSP_EBI_SRAM2_BUFPIN)
#define BSP_EBI_Sram1BuffOff() GPIO_PinOutSet(BSP_EBI_SRAM_BUFPORT, BSP_EBI_SRAM1_BUFPIN)
#define BSP_EBI_Sram2BuffOff() GPIO_PinOutSet(BSP_EBI_SRAM_BUFPORT, BSP_EBI_SRAM2_BUFPIN)

#define BSP_EBI_Sram1PowOn() GPIO_PinOutSet(BSP_EBI_SRAM_POWPORT, BSP_EBI_SRAM1_POWPIN)
#define BSP_EBI_Sram2PowOn() GPIO_PinOutSet(BSP_EBI_SRAM_POWPORT, BSP_EBI_SRAM2_POWPIN)
#define BSP_EBI_Sram1PowOff() GPIO_PinOutClear(BSP_EBI_SRAM_POWPORT, BSP_EBI_SRAM1_POWPIN)
#define BSP_EBI_Sram2PowOff() GPIO_PinOutClear(BSP_EBI_SRAM_POWPORT, BSP_EBI_SRAM2_POWPIN)

#define BSP_EBI_Sram1FPGAOn() GPIO_PinOutClear(BSP_EBI_EDAC_CONTROLPORT, BSP_EBI_EDAC_CONTROLPIN1)
#define BSP_EBI_Sram2FPGAOn() GPIO_PinOutClear(BSP_EBI_EDAC_CONTROLPORT, BSP_EBI_EDAC_CONTROLPIN2)
#define BSP_EBI_Sram1FPGAOff() GPIO_PinOutSet(BSP_EBI_EDAC_CONTROLPORT, BSP_EBI_EDAC_CONTROLPIN1)
#define BSP_EBI_Sram2FPGAOff() GPIO_PinOutSet(BSP_EBI_EDAC_CONTROLPORT, BSP_EBI_EDAC_CONTROLPIN2)
/** @} */

/***************************************************************************//**
 * @brief
 *   SRAM selector type definition.
 * @details
 *   Select between the SRAM modules to be used in the toggle functions \link
 *   BSP_EBI_setPower \endlink and \link BSP_EBI_setBuffer \endlink.
 ******************************************************************************/
typedef enum
{
  bspEbiSram1  = 0, ///< Select SRAM module 1.
  bspEbiSram2  = 1  ///< Select SRAM module 2.
} BSP_EBI_SRAMSelect_TypeDef;

void BSP_EBI_Init (void); ///< Initialises the CubeComputer's external bus interface.
uint8_t BSP_EBI_enableSRAM (BSP_EBI_SRAMSelect_TypeDef module);
void BSP_EBI_disableSRAM (BSP_EBI_SRAMSelect_TypeDef module);
void BSP_EBI_progEEPROM(uint32_t offset, uint8_t *buffer, uint8_t len); ///< Write a buffer to EEPROM.

/** @} (end addtogroup EBI) */
/** @} (end addtogroup BSP_Library) */

#endif // __BSP_EBI_H
