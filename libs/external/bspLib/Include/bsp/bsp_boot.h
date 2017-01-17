/***************************************************************************//**
 * @file	bsp_boot.h
 * @brief	BSP BOOT header file.
 *
 * This header file contains all the required definitions and function
 * prototypes through which to interface with the CubeComputer boot table
 * located in external EEPROM and flash.

 * @author	Pieter J. Botma
 * @date	15 October 2013
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

#ifndef BSP_BOOT_H_
#define BSP_BOOT_H_
#include <stdint.h>
#include "em_system.h"
#include "em_msc.h"
#include "bsp_dma.h"
#include "bsp_ebi.h"
#include "lld.h"
/***************************************************************************//**
 * @addtogroup BSP_Library
 * @brief Board Support Package (<b>BSP</b>) Driver Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup BOOT
 * @brief API for CubeComputer Boot Table interface.
 * @{
 ******************************************************************************/

/*************************** BOOT TABLE DESCRIPTION ****************************
 *
 * 0x14xxxxxx - FLASH Base Code Representation (see ref manual 14.3.12)
 * 0x84xxxxxx - FLASH Base Data Representation (see ref manual 14.3.12)
 *
 * 0x--00xxxx - Boot Table Entry 1
 * 0x----0000 - Boot Table Entry 1 Length
 * 0x----0020 - Boot Table Entry 1 CRC
 * 0x----0040 - Boot Table Entry 1 Valid Flag
 * 0x----0080 - Boot Table Entry 1 Description
 * 0x----0400 - Boot Table Entry 1 Code
 *
 * 0x--08xxxx - Boot Table Entry 2
 * .
 * .
 * .
 * 0x--38xxxx - Boot Table Entry 7
 *
 * 0x--3F0xxx - Boot Index (TMR)
 * 0x--3F2xxx - Boot Counter (TMR)
 * 0x--3FExxx - Flash test space
 *
 ******************************************************************************/
/**************************************************************************//**
 * @name Boot Table Defines
 *****************************************************************************/
//Flash configuration, default Bottom boot device.
uint32_t BOOT_TABLE_OFFSET_INDEX; ///< Offset for boot index
uint32_t BOOT_TABLE_OFFSET_CNTR; ///< Offset for boot counter
uint32_t BOOT_TABLE_OFFSET_CRC;///< Offset for current CRC
uint32_t BOOT_TABLE_OFFSET_TEST; ///< Offset for flash test space
uint32_t BOOT_TABLE_OFFSET_ENTRY1; ///< Offset for base of first table entry
/** @{ */
#define BOOT_TABLE_SIZE 													  7 ///< Number of entries in the boot table
#define BOOT_TABLE_BASE                								 0x84000000 ///< Address to the base boot table

#define BOOT_TABLE_ENTRY_SIZE									   (512 * 1024) ///< Space reserved for an entry in the boot table in bytes
#define BOOT_TABLE_OFFSET_ENTRYLENGTH           							  0 ///< Application length (add to entry offset)
#define BOOT_TABLE_OFFSET_ENTRYCRC              							 32 ///< Application CRC (add to entry offset)
#define BOOT_TABLE_OFFSET_ENTRYVALID          								 64 ///< Application valid flag (add to entry offset)
#define BOOT_TABLE_OFFSET_ENTRYDESRCIPTION									128 ///< Application description (add to entry offset)
#define BOOT_TABLE_OFFSET_ENTRYPROGRAM       							   1024 ///< Start of program (add to entry offset)
/** @} */

#define BOOT_SAFEMODE_BASE_CODE 									 0x12000000 ///< Address to the base of the safe mode EEPROM for code usage (MCU ref manual 14.3.12)
#define BOOT_SAFEMODE_BASE_DATA 									 0x80000000 ///< Address to the base of the safe mode EEPROM for data usage (MCU ref manual 14.3.12)

#define BOOT_TABLE_BASE_CODE										 0x14000000 ///< Address to the base boot table for code usage (MCU ref manual 14.3.12)
#define BOOT_TABLE_BASE_DATA										 0x84000000 ///< Address to the base boot table for code usage (MCU ref manual 14.3.12)

#define BOOT_BOOTLOADER_BASE										 0x00000000 ///< Address to the base of bootloader application
#define BOOT_BOOTLOADER_SIZE   									   (512 * 1024) ///< Size of the bootloader application in bytes

#define BOOT_APPLICATION_BASE										 0x00080000 ///< Address to the base of the main application
#define BOOT_APPLICATION_SIZE									   (512 * 1024) ///< Size of the main application in bytes

#define INT_FLASH_PAGE_SIZE												   4096 ///< Size of pages in internal flash in bytes
#define INT_FLASH_SECTOR_SIZE										 0x00010000 ///< Size of a sector in external flash in bytes

#define BOOT_CNTR_RESET 													  4 ///< Reset value of boot counter

#define BOOT_ENTRY_ISVALID												   0xAA ///< Entry valid flag value

#define BOOT_ENTRY_DESCRIPTION_SIZE											 64 ///< Entry description size

/**************************************************************************//**
 * @name Boot Table Entry Offset Defines
 *
 * Function defines for quickly determining offsets to boot table entry
 * parameters.
 *****************************************************************************/
/** @{ */

#define BOOT_getOffsetEntry(index)		 ( BOOT_TABLE_OFFSET_ENTRY1  + ( BOOT_TABLE_ENTRY_SIZE * ( index - 1 ) ) ) ///< Returns the offset from base to the entry in the boot table at the specified index
#define BOOT_getOffsetLen(index)	     ( BOOT_TABLE_OFFSET_ENTRY1  + ( BOOT_TABLE_ENTRY_SIZE * ( index - 1 ) ) + BOOT_TABLE_OFFSET_ENTRYLENGTH      ) ///< Returns the offset from base to the entry's length in the boot table at the specified index
#define BOOT_getOffsetCRC(index)	     ( BOOT_TABLE_OFFSET_ENTRY1  + ( BOOT_TABLE_ENTRY_SIZE * ( index - 1 ) ) + BOOT_TABLE_OFFSET_ENTRYCRC         ) ///< Returns the offset from base to the entry's CRC value in the boot table at the specified index
#define BOOT_getOffsetValid(index)		 ( BOOT_TABLE_OFFSET_ENTRY1  + ( BOOT_TABLE_ENTRY_SIZE * ( index - 1 ) ) + BOOT_TABLE_OFFSET_ENTRYVALID       ) ///< Returns the offset from base to the entry's valid flag in the boot table at the specified index
#define BOOT_getOffsetDescription(index) ( BOOT_TABLE_OFFSET_ENTRY1  + ( BOOT_TABLE_ENTRY_SIZE * ( index - 1 ) ) + BOOT_TABLE_OFFSET_ENTRYDESRCIPTION ) ///< Returns the offset from base to the entry's description in the boot table at the specified index
#define BOOT_getOffsetProgram(index)	 ( BOOT_TABLE_OFFSET_ENTRY1  + ( BOOT_TABLE_ENTRY_SIZE * ( index - 1 ) ) + BOOT_TABLE_OFFSET_ENTRYPROGRAM     ) ///< Returns the offset from base to the entry's program code in the boot table at the specified index
/** @} */

typedef enum {
	downloadSuccessful,
	indexNotValid,
	entryNotValid,
	entryCRCMismatch,
	downloadCRCMismatch
} BOOT_DownloadResult_Typedef;

uint8_t  BOOT_getBootIndex (void); 												///< Get current boot index
void     BOOT_setBootIndex (uint8_t index);										///< Set current boot index
uint8_t  BOOT_getBootCounter (void);											///< Get current boot counter
void     BOOT_decBootCounter (void);											///< Decrement current boot counter
void     BOOT_resetBootCounter (void);											///< Reset current boot counter
uint32_t BOOT_getLen (uint8_t index);											///< Get length of entry at specified index
void     BOOT_setLen (uint8_t index, uint32_t len);								///< Set length of entry at specified index
uint16_t BOOT_getCRC (uint8_t index);											///< Get CRC value of entry at specified index
void     BOOT_setCRC (uint8_t index, uint16_t crc);								///< Set CRC value of entry at specified index
uint16_t BOOT_calcCRC(uint8_t *start, uint8_t *end);							///< This function calculates the CRC-16-CCIT checksum of a memory range.
uint8_t  BOOT_getValid (uint8_t index);											///< Get valid flag of entry at specified index
void     BOOT_setValid (uint8_t index);											///< Set valid flag of entry at specified index
void     BOOT_clearValid (uint8_t index);										///< Clear valid flag of entry at specified index
void     BOOT_programDescription (uint8_t index, uint8_t* buffer);				///< Update description of entry at specified index
void     BOOT_programEntry (uint8_t index, uint32_t offset, uint8_t *buffer, uint32_t len); ///< Program new entry in boot table at specified index.
void	 BOOT_eraseEntry (uint8_t index);										///< Erase flash sectors at specified entry in boot table
void     BOOT_boot (uint32_t bootAddress);										///< Sets up new vector table before booting into application space.
BOOT_DownloadResult_Typedef BOOT_tryDownloadEntryToApplicationSpace(uint8_t index); ///< Download an entry from boot table into MCU application space.
void     BOOT_jump (uint32_t sp, uint32_t pc);									///< Jump execution from boot space to application space

/** @} (end addtogroup BOOT) */
/** @} (end addtogroup BSP_Library) */

#endif /* BSP_BOOT_H_ */
