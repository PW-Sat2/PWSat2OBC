/***************************************************************************/ /**
  * @file	bsp_can.c
  * @brief	BSP CAN source file.
  *
  * This file contains all the implementations for the functions defined in \em
  * bsp_boot.h.
  * @author	Pieter J. Botma
  * @date	15 Oct 2013
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

/***************************************************************************/ /**
  * @addtogroup BSP_Library
  * @brief Board Support Package (<b>BSP</b>) Driver Library for CubeComputer.
  * @{
  ******************************************************************************/

/***************************************************************************/ /**
  * @addtogroup BOOT
  * @brief API for CubeComputer Boot Table interface.
  * @{
  ******************************************************************************/
#include "bsp_boot.h"
#include <core_cmFunc.h>
// Global variables
DMA_DESCRIPTOR_TypeDef descr __attribute__((aligned(256)));

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

// Forward declaration of helper functions
uint8_t entryCRCVerifiedFor(uint8_t entryIndex);
uint8_t downloadedCRCVerifiedFor(uint8_t entryIndex);
void downloadEntryToApplicationSpace(uint8_t index) __attribute__((section(".ram")));
void downloadBlockToApplicationSpace(void* base, uint32_t size, uint8_t const* data) __attribute__((section(".ram")));
void jumpExecutionTo(uint32_t sp, uint32_t pc);

/** @endcond */

/**************************************************************************/ /**
  * @author	Pieter J. Botma
  * @date	17/03/2014
  *
  * This function tries to load an entry in the boot table into the MCU
  * application space. This if the following are true:
  *   - The entry is valid
  *   - The actual and expected CRCs match.
  *   - The downloaded and expected CRCs match.
  *
  * @param [in] index
  *   Index of entry for which sectors should be erased.
  *
  * @return
  *   Result of download attempt.
  *
  *****************************************************************************/
BOOT_DownloadResult_Typedef BOOT_tryDownloadEntryToApplicationSpace(uint8_t entryIndex)
{
    BOOT_DownloadResult_Typedef result;

    if ((entryIndex > 0) && (entryIndex <= BOOT_TABLE_SIZE))
    {
        if (BOOT_getValid(entryIndex) == BOOT_ENTRY_ISVALID)
        {
            if (entryCRCVerifiedFor(entryIndex))
            {
                downloadEntryToApplicationSpace(entryIndex);

                if (downloadedCRCVerifiedFor(entryIndex))
                {
                    result = downloadSuccessful;
                }
                else
                {
                    result = downloadCRCMismatch;
                }
            }
            else
            {
                result = entryCRCMismatch;
            }
        }
        else
        {
            result = entryNotValid;
        }
    }
    else
    {
        result = indexNotValid;
    }

    return result;
}

uint8_t entryCRCVerifiedFor(uint8_t entryIndex)
{
    uint8_t *startAddr, *endAddr;
    uint16_t expectedCRC, actualCRC;

    startAddr = ((uint8_t*)BOOT_TABLE_BASE) + BOOT_getOffsetProgram(entryIndex);
    endAddr = (uint8_t*)(startAddr + BOOT_getLen(entryIndex));

    actualCRC = BOOT_calcCRC(startAddr, endAddr);
    expectedCRC = BOOT_getCRC(entryIndex);

    return (actualCRC == expectedCRC);
}

uint8_t downloadedCRCVerifiedFor(uint8_t entryIndex)
{
    uint8_t *startAddr, *endAddr;
    uint16_t expectedCRC, actualCRC;

    startAddr = (uint8_t*)BOOT_APPLICATION_BASE;
    endAddr = (uint8_t*)(startAddr + BOOT_getLen(entryIndex));

    actualCRC = BOOT_calcCRC(startAddr, endAddr);
    expectedCRC = BOOT_getCRC(entryIndex);

    return (actualCRC == expectedCRC);
}

void downloadEntryToApplicationSpace(uint8_t index)
{
    uint32_t entryOffset, entryLength, blockBase, blockSize;
    uint8_t* blockData;

    // initialize internal memory
    MSC->LOCK = MSC_UNLOCK_CODE;
    MSC->WRITECTRL |= MSC_WRITECTRL_WREN | MSC_WRITECTRL_RWWEN;

    // erase application space
    MSC->MASSLOCK = MSC_MASSLOCK_LOCKKEY_UNLOCK;
    MSC->WRITECMD |= MSC_WRITECMD_ERASEMAIN1;
    MSC->MASSLOCK = MSC_MASSLOCK_LOCKKEY_LOCK;

    while ((MSC->STATUS & MSC_STATUS_BUSY))
        ;

    entryLength = BOOT_getLen(index);

    // write data to flash in blocks the size of a flash page
    for (entryOffset = 0; entryOffset < entryLength; entryOffset += INT_FLASH_PAGE_SIZE)
    {
        blockBase = BOOT_APPLICATION_BASE + entryOffset;
        blockSize = ((entryOffset + INT_FLASH_PAGE_SIZE) < entryLength) ? INT_FLASH_PAGE_SIZE : (entryLength - entryOffset);
        blockData = (uint8_t*)(BOOT_TABLE_BASE + BOOT_getOffsetProgram(index) + entryOffset);

        downloadBlockToApplicationSpace((void*)(blockBase), blockSize, blockData);

        while ((MSC->STATUS & MSC_STATUS_BUSY))
            ;
    }
}

void downloadBlockToApplicationSpace(void* base, uint32_t size, uint8_t const* data)
{
    uint32_t i;

    MSC->ADDRB = ((uint32_t)base);
    MSC->WRITECMD = MSC_WRITECMD_LADDRIM;

    MSC->WDATA = *((uint32_t*)(data + 0));
    MSC->WRITECMD = MSC_WRITECMD_WRITETRIG;

    for (i = 4; i < size; i += 4)
    {
        while (!(MSC->STATUS & MSC_STATUS_WDATAREADY))
            ;

        MSC->WDATA = *((uint32_t*)(data + i));
    }

    while (!(MSC->STATUS & MSC_STATUS_WDATAREADY))
        ;

    MSC->WRITECMD = MSC_WRITECMD_WRITEEND;
}

/**************************************************************************/ /**
  * @author	Pieter J. Botma
  * @date	13/03/2014
  *
  * This function sets the vector table to the specified boot address, reads the
  * new program counter and stack pointer from the new vector table and starts
  * execution.
  *
  *****************************************************************************/
void BOOT_boot(uint32_t bootAddress)
{
    // Set new vector table
    SCB->VTOR = bootAddress;

    // Read new SP and PC from vector table
    uint32_t sp = *((uint32_t*)bootAddress);
    uint32_t pc = *((uint32_t*)bootAddress + 1);

    asm volatile("ldr r0, %0\n"
                 "ldr r1, %1\n"
                 "MSR msp, r0\n"
                 "MSR psp, r0\n"
                 "mov pc, r1\n" ::"m"(sp),
                 "m"(pc)
                 : "sp");
}

/**************************************************************************/ /**
  * @author	Pieter J. Botma
  * @date	16/06/2013
  *
  * This function returns the current boot index, indicating the entry in the
  * boot table to be executed next.
  *
  * @return
  *   The current boot index.
  *
  *****************************************************************************/
uint8_t BOOT_getBootIndex(void)
{
    return *((uint8_t*)(BOOT_TABLE_BASE + BOOT_TABLE_OFFSET_INDEX));
}

/**************************************************************************/ /**
  * @author	Pieter J. Botma
  * @date	16/06/2013
  *
  * This function sets the current boot index, indicating the entry in the
  * boot table to be executed next.
  *
  * @param [in] index
  *   Value of new boot index
  *
  *****************************************************************************/
void BOOT_setBootIndex(uint8_t index)
{
    lld_SectorEraseOp((uint8_t*)(BOOT_TABLE_BASE), BOOT_TABLE_OFFSET_INDEX);
    lld_ProgramOp((uint8_t*)BOOT_TABLE_BASE, BOOT_TABLE_OFFSET_INDEX, index);
}

/**************************************************************************/ /**
  * @author	Pieter J. Botma
  * @date	16/06/2013
  *
  * This function returns the current boot counter, indicating the amount of
  * resets allowed before booting to safe mode
  *
  * @return
  *   The current boot counter.
  *
  *****************************************************************************/
uint8_t BOOT_getBootCounter(void)
{
    return *((uint8_t*)(BOOT_TABLE_BASE + BOOT_TABLE_OFFSET_CNTR));
}

/**************************************************************************/ /**
  * @author	Pieter J. Botma
  * @date	16/06/2013
  *
  * This function decrements the current boot counter, decreasing the amount of
  * resets allowed before booting to safe mode
  *
  *****************************************************************************/
void BOOT_decBootCounter(void)
{
    uint16_t cntr = BOOT_getBootCounter();

    lld_SectorEraseOp((uint8_t*)(BOOT_TABLE_BASE), BOOT_TABLE_OFFSET_CNTR);
    lld_ProgramOp((uint8_t*)BOOT_TABLE_BASE, BOOT_TABLE_OFFSET_CNTR, (cntr >> 1));
}

/**************************************************************************/ /**
  * @author	Pieter J. Botma
  * @date	16/06/2013
  *
  * This function resets the current boot counter, indicating the amount of
  * resets allowed before booting to safe mode.
  *
  *****************************************************************************/
void BOOT_resetBootCounter(void)
{
    lld_SectorEraseOp((uint8_t*)(BOOT_TABLE_BASE), BOOT_TABLE_OFFSET_CNTR);
    lld_ProgramOp((uint8_t*)BOOT_TABLE_BASE, BOOT_TABLE_OFFSET_CNTR, BOOT_CNTR_RESET);
}

/**************************************************************************/ /**
  * @author	Pieter J. Botma
  * @date	16/06/2013
  *
  * This function returns the length of an application stored in the boot table
  * at the specified index.
  *
  * @param [in] index
  *   Index of entry for which length should be returned.
  * @return
  *   The length of the application at the specified index.
  *
  *****************************************************************************/
uint32_t BOOT_getLen(uint8_t index)
{
    return *((uint32_t*)(BOOT_TABLE_BASE + BOOT_getOffsetLen(index)));
}

/**************************************************************************/ /**
  * @author	Pieter J. Botma
  * @date	16/06/2013
  *
  * This function sets the length of an application stored in the boot table
  * at the specified index.
  *
  * @param [in] index
  *   Index of entry for which length should be set.
  * @param [in] len
  *   Length value to be saved.
  *
  *****************************************************************************/
void BOOT_setLen(uint8_t index, uint32_t len)
{
    lld_ProgramOp((uint8_t*)BOOT_TABLE_BASE, (BOOT_getOffsetLen(index)), (uint8_t)(len));
    lld_ProgramOp((uint8_t*)BOOT_TABLE_BASE, (BOOT_getOffsetLen(index) + 1), (uint8_t)(len >> 8));
    lld_ProgramOp((uint8_t*)BOOT_TABLE_BASE, (BOOT_getOffsetLen(index) + 2), (uint8_t)(len >> 16));
    lld_ProgramOp((uint8_t*)BOOT_TABLE_BASE, (BOOT_getOffsetLen(index) + 3), (uint8_t)(len >> 24));
}

/**************************************************************************/ /**
  * @author	Pieter J. Botma
  * @date	16/06/2013
  *
  * This function returns the CRC value of the entry at the specified index.
  *
  * @param [in] index
  *   Index of entry for which CRC should be returned.
  * @return
  *   The CRC of the application at the specified index.
  *
  *****************************************************************************/
uint16_t BOOT_getCRC(uint8_t index)
{
    return *((uint16_t*)(BOOT_TABLE_BASE + BOOT_getOffsetCRC(index)));
}

/**************************************************************************/ /**
  * @author	Pieter J. Botma
  * @date	16/06/2013
  *
  * This function sets the CRC value of the entry at the specified index.
  *
  * @param [in] index
  *   Index of entry for which CRC should be set.
  * @param [in] crc
  *   CRC value to be saved.
  *
  *****************************************************************************/
void BOOT_setCRC(uint8_t index, uint16_t crc)
{
    lld_ProgramOp((uint8_t*)BOOT_TABLE_BASE, (BOOT_getOffsetCRC(index)), (uint8_t)(crc));
    lld_ProgramOp((uint8_t*)BOOT_TABLE_BASE, (BOOT_getOffsetCRC(index) + 1), (uint8_t)(crc >> 8));
}

/**************************************************************************/ /**
  * @author  Energy Micro AS
  * @version 1.63
  *
  * This function calculates the CRC-16-CCIT checksum of a memory range.
  *
  * @note
  *   This implementation uses an initial value of 0, while some implementations
  *   of CRC-16-CCIT uses an initial value of 0xFFFF. If you wish to
  *   precalculate the CRC before uploading the binary to the bootloader you
  *   can use this function. However, keep in mind that the 'v' and 'c' commands
  *   computes the crc of the entire flash, so any bytes not used by your
  *   application will have the value 0xFF.
  *
  * @param start
  *   Pointer to the start of the memory block
  *
  * @param end
  *   Pointer to the end of the block. This byte is not included in the computed
  *   CRC.
  *
  * @return
  *   The computed CRC value.
  *****************************************************************************/
uint16_t BOOT_calcCRC(uint8_t* start, uint8_t* end)
{
    uint16_t crc = 0x0;
    uint8_t* data;

    for (data = start; data < end; data++)
    {
        crc = (crc >> 8) | (crc << 8);
        crc ^= *data;
        crc ^= (crc & 0xff) >> 4;
        crc ^= crc << 12;
        crc ^= (crc & 0xff) << 5;
    }
    return crc;
}

/**************************************************************************/ /**
  * @author	Pieter J. Botma
  * @date	16/06/2013
  *
  * This function returns the valid flag of an entry at a specified index in the
  * boot table. The valid flag indicates whether the entry in the boot table
  * holds a valid application.
  *
  * @param [in] index
  *   Index of entry for which valid flag should be returned.
  * @return
  *   Flag indicating whether the application at the specified index is valid.
  *
  *****************************************************************************/
uint8_t BOOT_getValid(uint8_t index)
{
    return *((uint16_t*)(BOOT_TABLE_BASE + BOOT_getOffsetValid(index)));
}

/**************************************************************************/ /**
  * @author	Pieter J. Botma
  * @date	16/06/2013
  *
  * This function sets the valid flag of an entry at a specified index in the
  * boot table.
  *
  * @note
  *   The entry is valid if it is equal to a specific value, not 0xFF.
  *
  * @param [in] index
  *   Index of entry for which the valid flag should be set.
  *
  *****************************************************************************/
void BOOT_setValid(uint8_t index)
{
    lld_ProgramOp((uint8_t*)BOOT_TABLE_BASE, (BOOT_getOffsetValid(index)), (BOOT_ENTRY_ISVALID));
}

/**************************************************************************/ /**
  * @author	Pieter J. Botma
  * @date	16/06/2013
  *
  * This function clears the valid flag of an entry at a specified index in the
  * boot table.
  *
  * @param [in] index
  *   Index of entry for which the valid flag should be cleared.
  *
  *****************************************************************************/
void BOOT_clearValid(uint8_t index)
{
    lld_ProgramOp((uint8_t*)BOOT_TABLE_BASE, (BOOT_getOffsetValid(index)), (0x00));
}

/**************************************************************************/ /**
  * @author	Pieter J. Botma
  * @date	17/03/2014
  *
  * This function writes a description for an entry at the specified index in
  * the boot table.
  *
  * @param [in] index
  *   Index of entry for which description should be update.
  *
  *****************************************************************************/
void BOOT_programDescription(uint8_t index, uint8_t* buffer)
{
    uint8_t* base;
    uint32_t offset, i;

    // Write data to external FLASH, i.e. Nominal Mode
    if (index >= 1 && index <= BOOT_TABLE_SIZE)
    {
        base = (uint8_t*)BOOT_TABLE_BASE;
        offset = BOOT_getOffsetDescription(index);

        // Write to flash (write 1 byte at a time)
        for (i = 0; i < BOOT_ENTRY_DESCRIPTION_SIZE; i++)
        {
            lld_ProgramOp(base, (offset + i), buffer[i]);

            if (buffer[i] == '\n')
            {
                break;
            }
        }
    }
}

/**************************************************************************/ /**
  * @author	Pieter J. Botma
  * @date	11/12/2013
  *
  * This function erases the flash sectors assigned to hold the entry in boot
  * table at the specified boot index.
  *
  * @param [in] index
  *   Index of entry for which sectors should be erased.
  *
  *****************************************************************************/
void BOOT_eraseEntry(uint8_t index)
{
    uint32_t offset;

    // Erase sectors in boot table for specified index
    if (index == 0)
    {
        return; // error! invalid index
    }
    else if (index > BOOT_TABLE_SIZE)
    {
        return; // error! invalid index
    }
    else
    {
        for (offset = 0; offset < BOOT_TABLE_ENTRY_SIZE; offset += BSP_EBI_FLASH_LSECTOR_SIZE)
            lld_SectorEraseOp((uint8_t*)(BOOT_TABLE_BASE), (BOOT_getOffsetEntry(index) + offset));
    }
}

/**************************************************************************/ /**
  * @author	Pieter J. Botma
  * @date	11/12/2013
  *
  * This function writes a buffer of data of specified length into the boot table
  * at the specified index.
  *
  * @param [in] index
  *   Index of entry for which data should be written.
  * @param [in] offset
  *   Offset into entry at which data should be written.
  * @param [in] buffer
  *   Buffer of data to be written into boot table.
  * @param [in] len
  *   Length of data to be written into boot table.
  *
  *****************************************************************************/
void BOOT_programEntry(uint8_t index, uint32_t offset, uint8_t* buffer, uint32_t len)
{
    uint8_t* base;
    uint32_t i;

    // Write data to external EEPROM, i.e. Safe Mode
    if (index == 0)
    {
        i = 0;
        base = (uint8_t*)(BOOT_SAFEMODE_BASE_DATA + offset);

        do
        {
            // Unlock commands
            *((uint8_t*)(BOOT_SAFEMODE_BASE_DATA + 0x5555)) = 0xAA;
            *((uint8_t*)(BOOT_SAFEMODE_BASE_DATA + 0x2AAA)) = 0x55;
            *((uint8_t*)(BOOT_SAFEMODE_BASE_DATA + 0x5555)) = 0xA0;

            // Write to page (can only write in 64 bytes at a time)
            do
            {
                *(base + i) = buffer[i];
                i++;
            } while ((i < len) && ((i % 64) != 0));

            // Poll write sequence completion
            while (((*(base + i - 1)) & 0x80) != (buffer[i - 1] & 0x80))
                ;
        } while (i < len);
    }
    // Write data to external FLASH, i.e. Nominal Mode
    else if (index < BOOT_TABLE_SIZE)
    {
        base = (uint8_t*)BOOT_TABLE_BASE;
        offset += BOOT_getOffsetProgram(index);

        // Write to flash (write 1 byte at a time)
        for (i = 0; i < len; i++)
        {
            lld_ProgramOp(base, (offset + i), buffer[i]);
        }
    }
    else
    {
        // error!
    }
}

/** @} (end addtogroup BOOT) */
/** @} (end addtogroup BSP_Library) */
