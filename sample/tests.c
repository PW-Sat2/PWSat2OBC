/***************************************************************************/ /**
  * @file
  * @brief	CubeComputer Test Program Test Implementations.
  *
  * This file contains the implementations for functions written to test the
  * functionality of all the peripherals and subsystems of CubeComputer.
  *
  * @author	Pieter J. Botma
  * @date	04/04/2013
  *
  ******************************************************************************/

#include "tests.h"

#define EEPROM_TestSize 64
#define TEST_ARRAY_SIZE 32

#define testValue(addr) ((uint8_t)((addr * 7) % 255))

extern uint8_t TopBoot;
extern uint8_t BotBoot;

void TEST_EEPROM(void);
void TEST_EEPROM_COMPLETE(void);
void TEST_FLASH(void);
void TEST_FLASH_COMPLETE(void);
void TEST_SRAM(void);
void PerformSRAMtest();
uint8_t FlashTestSeg(uint32_t offset, uint32_t segsize);

uint8_t testResultEBI[3]; // 0 = EEPROM test result, 1 = FLASH test results, 2 = SRAM

extern uint32_t BOOT_TABLE_OFFSET_INDEX;  ///< Offset for boot index
extern uint32_t BOOT_TABLE_OFFSET_CNTR;   ///< Offset for boot counter
extern uint32_t BOOT_TABLE_OFFSET_CRC;    ///< Offset for current CRC
extern uint32_t BOOT_TABLE_OFFSET_TEST;   ///< Offset for flash test space
extern uint32_t BOOT_TABLE_OFFSET_ENTRY1; ///< Offset for base of first table entry
//*** MSD Defines ***//
#define BUFFERSIZE 512           // BUFFERSIZE should be between 512 and 1024, depending on available ram on efm32
#define TEST_FILENAME "test.txt" // Filename to open/write/read from SD-card
uint8_t testResultMicroSD;       // Stores Micro SD card test results

/******************************* RTC ***************************************/ /**
  *
  * @author Pieter J. Botma
  * @date   04/04/2013
  *
  * This function outputs the seconds counter over the debug UART.
  *
  ******************************************************************************/
void TEST_RTC(void)
{
    uint8_t i = 0;
    for (i = 0; i < 10; i++)
    {
        // Obtain current time as seconds elapsed since Epoch
        time_t current_time = time(NULL);

        if (current_time == (time_t)-1)
        {
            // error
        }

        debugLen = sprintf((char*)debugStr, "\n\nTime:");
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

        debugLen = sprintf((char*)debugStr, "\nBURTC: %ld", BURTC_CounterGet());
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

        debugLen = sprintf((char*)debugStr, "\nUnix Time: %d", (int)time(NULL));
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

        debugLen = sprintf((char*)debugStr, "\nTime: %s", ctime(&current_time)); // Convert to local time format
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

        debugLen = sprintf((char*)debugStr, "Runtime: %d ms", (int)msTicks);
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

        Delay(1000); // delay for 1 second (1000ms)
    }
}

/******************************* ADC ***************************************/ /**
  *
  * @author Pieter J. Botma
  * @date   04/04/2013
  *
  * This function samples, via ADC, and displays the OBC current and voltage
  * values over the debug UART.
  *
  ******************************************************************************/
void TEST_ADC(void)
{
    int temp;
    debugLen = sprintf((char*)debugStr, "\n\nTesting ADCs:");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

    BSP_ADC_update(1);
    temp = BSP_ADC_getData(TEMPERATURE) / 256;
    debugLen = sprintf((char*)debugStr,
        "\nChannel 0 (mV): %d\nChannel 1 (mV): %d\nChannel 2 (mV): %d\nChannel 3 (mV): %d\nCelcius (C): %d",
        BSP_ADC_getData(0),
        BSP_ADC_getData(1),
        BSP_ADC_getData(2),
        BSP_ADC_getData(3),
        temp);
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
}

void TEST_EEPROM(void)
{
    uint32_t i = 0;
    uint32_t FailMark = 55;
    uint8_t test[TEST_ARRAY_SIZE] = {0x7B,
        0x1E,
        0x2E,
        0x9F,
        0xE9,
        0x7E,
        0x73,
        0x17,
        0xAE,
        0x8C,
        0x1E,
        0xAC,
        0x9E,
        0x6F,
        0x45,
        0x8E,
        0x30,
        0x1C,
        0xA3,
        0xE4,
        0xE5,
        0xC1,
        0x1A,
        0x52,
        0xF6,
        0x24,
        0xDF,
        0x9B,
        0xAD,
        0x41,
        0xE6,
        0x37};
    uint8_t answer[TEST_ARRAY_SIZE];

    testResultEBI[0] = 1;

    // clear answer array
    for (i = 0; i < TEST_ARRAY_SIZE; i++)
    {
        answer[i] = 0xFF;
    }

    // Write data in the External EEPROM
    BSP_EBI_progEEPROM(128, test, TEST_ARRAY_SIZE);

    // Read external EEPROM
    for (i = 0; i < TEST_ARRAY_SIZE; i++)
    {
        answer[i] = *(uint8_t*)(BSP_EBI_EEPROM_BASE + i + 128);
    }

    // Test the difference between buffers.
    for (i = 0; i < TEST_ARRAY_SIZE; i++)
    {
        if (test[i] != answer[i])
        {
            testResultEBI[0] = 0;
            if (FailMark == 55)
            {
                FailMark = i;
            }
        }
    }

    if (testResultEBI[0] == 0)
    {
        // Write and Read operation FAILED
        debugLen = sprintf((char*)debugStr, "\nEEPROM test failed! %d,%d", FailMark, i);
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    }
    else
    {
        // Write and Read operation SUCCESS
        debugLen = sprintf((char*)debugStr, "\nEEPROM test successful!");
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    }
}

void TEST_EEPROM_COMPLETE(void)
{
    uint32_t i, j = 0;
    uint8_t EEPROM_TestValues[EEPROM_TestSize] = {0};
    uint8_t EEPROM_TestAnwsers[EEPROM_TestSize] = {0};

    // Populate the EEPROM test array
    for (i = 0; i < EEPROM_TestSize; i++)
    {
        EEPROM_TestValues[i] = testValue(i);
    }

    debugLen = sprintf((char*)debugStr, "\n\nRunning entire EEPROM test");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

    testResultEBI[0] = 1; // Reset the result value

    // Write data in the External EEPROM
    for (i = 0; i < (BSP_EBI_EEPROM_SIZE / EEPROM_TestSize); i++)
    {
        BSP_EBI_progEEPROM((i * EEPROM_TestSize), EEPROM_TestValues, EEPROM_TestSize);
        Delay(1);
    }

    // Read External EEPROM
    for (i = 0; i < (BSP_EBI_EEPROM_SIZE / EEPROM_TestSize); i++)
    {
        for (j = 0; j < EEPROM_TestSize; j++)
        {
            EEPROM_TestAnwsers[j] = *(uint8_t*)(BSP_EBI_EEPROM_BASE + j + (i * EEPROM_TestSize));
        }
        // Test the difference
        for (j = 0; j < EEPROM_TestSize; j++)
        {
            if (EEPROM_TestAnwsers[j] != EEPROM_TestValues[j])
            {
                testResultEBI[0] = 0;
            }
        }
    }

    if (testResultEBI[0] == 0)
    {
        // Write and Read operation FAILED
        debugLen = sprintf((char*)debugStr, "\nEEPROM test failed!");
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    }
    else
    {
        // Write and Read operation SUCCESS
        debugLen = sprintf((char*)debugStr, "\nEEPROM test successful!");
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    }
}

void TEST_FLASH()
{
    uint32_t i = 0;
    uint32_t FailMark = 55;
    uint8_t status;
    /* Global variables */
    uint8_t test[TEST_ARRAY_SIZE] = {0x7B,
        0x1E,
        0x2E,
        0x9F,
        0xE9,
        0x7E,
        0x73,
        0x17,
        0xAE,
        0x8C,
        0x1E,
        0xAC,
        0x9E,
        0x6F,
        0x45,
        0x8E,
        0x30,
        0x1C,
        0xA3,
        0xE4,
        0xE5,
        0xC1,
        0x1A,
        0x52,
        0xF6,
        0x24,
        0xDF,
        0x9B,
        0xAD,
        0x41,
        0xE6,
        0x37};

    uint8_t answer[TEST_ARRAY_SIZE];

    testResultEBI[1] = 1;

    for (i = 0; i < TEST_ARRAY_SIZE; i++) // clear answer array
    {
        answer[i] = 0xFF;
    }
    debugLen = sprintf((char*)debugStr, "\n\nRunning FLASH test:");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

    status = lld_SectorEraseOp((uint8_t*)BOOT_TABLE_BASE, BOOT_TABLE_OFFSET_TEST);

    // Write external FLASH
    for (i = 0; i < TEST_ARRAY_SIZE; i++)
    {
        status = lld_ProgramOp((uint8_t*)(BOOT_TABLE_BASE), (BOOT_TABLE_OFFSET_TEST + i), test[i]);
    }

    // Read external FLASH
    for (i = 0; i < TEST_ARRAY_SIZE; i++)
    {
        answer[i] = *(uint8_t*)(BOOT_TABLE_BASE + BOOT_TABLE_OFFSET_TEST + i);
    }

    // Test the difference between buffers.
    for (i = 0; i < TEST_ARRAY_SIZE; i++)
    {
        if (test[i] != answer[i])
        {
            testResultEBI[1] = 0;
            if (FailMark == 55)
            {
                FailMark = i;
            }
        }
    }

    if (testResultEBI[1] == 0)
    {
        // Write and Read operation  FAILED
        debugLen = sprintf((char*)debugStr, "\nFLASH test failed! %d, %d", FailMark, (int)i);
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    }
    else
    {
        // Write and Read operation  SUCCESS
        debugLen = sprintf((char*)debugStr, "\nFLASH test successful!");
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    }
}

void TEST_FLASH_COMPLETE()
{
    unsigned int FlashID = 0;
    unsigned int FlashBootConfig = 0;

    uint8_t testResult = 0;

    debugLen = sprintf((char*)debugStr, "\n\nRunning entire FLASH test:");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

    FlashID = lld_GetDeviceId((uint8_t*)0x84000000);
    FlashBootConfig = lld_ReadCfiWord((uint8_t*)0x84000000, 0x9E);

    // Read the Flash ID
    // debugLen = sprintf((char*)debugStr,"\nFlash id = %8.8X", lld_GetDeviceId((uint8_t*) 0x84000000));
    debugLen = sprintf((char*)debugStr, "\nFlash id = %8.8X", FlashID);
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

    // Read the Flash Boot configuration
    // debugLen = sprintf((char*)debugStr,"\nFlash Boot type = %8.8X",lld_ReadCfiWord(0x84000000,0x9E));
    debugLen = sprintf((char*)debugStr, "\nFlash Boot type = %8.8X", FlashBootConfig);
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

    FlashID = (FlashID & 0x00FF0000);
    switch (FlashID)
    {
        case 0x00F60000:                       // S29AL032D device Model 03
            if (FlashBootConfig == 0x00000003) // Model 03 Top Boot Device
            {
                debugLen = sprintf((char*)debugStr, "\nFlash model: S29AL032D Top Boot");
                BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
            }
            else if (FlashBootConfig == 0x00000002) // Model 04 Bottom Boot Device
            {
                debugLen = sprintf((char*)debugStr, "\nFlash model: S29AL032D Bottom Boot");
                BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
            }
            break;

        case 0x00F90000:                       // S29AL032D device Model 04
            if (FlashBootConfig == 0x00000003) // Model 03 Top Boot Device
            {
                debugLen = sprintf((char*)debugStr, "\nFlash model: S29AL032D Top Boot");
                BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
            }
            else if (FlashBootConfig == 0x00000002) // Model 04 Bottom Boot Device
            {
                debugLen = sprintf((char*)debugStr, "\nFlash model: S29AL032D Bottom Boot");
                BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
            }
            break;

        case 0x00000000:                       // S29JL032H device or S29JL032J device Model 01 or 02
            if (FlashBootConfig == 0x00000003) // 03 = Top Boot device
            {
                debugLen = sprintf((char*)debugStr, "\nFlash model: S29JL032H or S29JL032J Top Boot");
                BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
            }
            else if (FlashBootConfig == 0x00000002) // 02 Bottom Boot Device
            {
                debugLen = sprintf((char*)debugStr, "\nFlash model: S29JL032H or S29JL032J Bottom Boot");
                BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
            }
            break;

        case 0x00010000:                       // S29JL032H device or S29JL032J device Model 01 or 02
            if (FlashBootConfig == 0x00000003) // 03 = Top Boot device
            {
                debugLen = sprintf((char*)debugStr, "\nFlash model: S29JL032H or S29JL032J Top Boot");
                BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
            }
            else if (FlashBootConfig == 0x00000002) // 02 Bottom Boot Device
            {
                debugLen = sprintf((char*)debugStr, "\nFlash model: S29JL032H or S29JL032J Bottom Boot");
                BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
            }
            break;

        case 0x00560000:                       // S29JL032H device or S29JL032J device Model 21 or 22
            if (FlashBootConfig == 0x00000003) // 03 = Top Boot device
            {
                debugLen = sprintf((char*)debugStr, "\nFlash model: S29JL032H or S29JL032J Top Boot");
                BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
            }
            else if (FlashBootConfig == 0x00000002) // 02 Bottom Boot Device
            {
                debugLen = sprintf((char*)debugStr, "\nFlash model: S29JL032H or S29JL032J Bottom Boot");
                BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
            }
            break;

        case 0x00550000:                       // S29JL032H device or S29JL032J device Model 21 or 22
            if (FlashBootConfig == 0x00000003) // 03 = Top Boot device
            {
                debugLen = sprintf((char*)debugStr, "\nFlash model: S29JL032H or S29JL032J Top Boot");
                BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
            }
            else if (FlashBootConfig == 0x00000002) // 02 Bottom Boot Device
            {
                debugLen = sprintf((char*)debugStr, "\nFlash model: S29JL032H or S29JL032J Bottom Boot");
                BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
            }
            break;

        case 0x00530000:                       // S29JL032H device or S29JL032J device Model 31 or 32
            if (FlashBootConfig == 0x00000003) // 03 = Top Boot device
            {
                debugLen = sprintf((char*)debugStr, "\nFlash model: S29JL032H or S29JL032J Top Boot");
                BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
            }
            else if (FlashBootConfig == 0x00000002) // 02 Bottom Boot Device
            {
                debugLen = sprintf((char*)debugStr, "\nFlash model: S29JL032H or S29JL032J Bottom Boot");
                BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
            }
            break;

        case 0x00500000:                       // S29JL032H device or S29JL032J device Model 31 or 32
            if (FlashBootConfig == 0x00000003) // 03 = Top Boot device
            {
                debugLen = sprintf((char*)debugStr, "\nFlash model: S29JL032H or S29JL032J Top Boot");
                BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
            }
            else if (FlashBootConfig == 0x00000002) // 02 Bottom Boot Device
            {
                debugLen = sprintf((char*)debugStr, "\nFlash model: S29JL032H or S29JL032J Bottom Boot");
                BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
            }
            break;

        case 0x005F0000:                       // S29JL032H device or S29JL032J device Model 41 or 42
            if (FlashBootConfig == 0x00000003) // 03 = Top Boot device
            {
                debugLen = sprintf((char*)debugStr, "\nFlash model: S29JL032H or S29JL032J Top Boot");
                BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
            }
            else if (FlashBootConfig == 0x00000002) // 02 Bottom Boot Device
            {
                debugLen = sprintf((char*)debugStr, "\nFlash model: S29JL032H or S29JL032J Bottom Boot");
                BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
            }
            break;

        case 0x005C0000:                       // S29JL032H device or S29JL032J device Model 41 or 42
            if (FlashBootConfig == 0x00000003) // 03 = Top Boot device
            {
                debugLen = sprintf((char*)debugStr, "\nFlash model: S29JL032H or S29JL032J Top Boot");
                BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
            }
            else if (FlashBootConfig == 0x00000002) // 02 Bottom Boot Device
            {
                debugLen = sprintf((char*)debugStr, "\nFlash model: S29JL032H or S29JL032J Bottom Boot");
                BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
            }
            break;

        default:
            debugLen = sprintf((char*)debugStr, "\nUnknown Flash device");
            BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
            break;
    }
    testResult = testResult + FlashTestSeg(BOOT_TABLE_OFFSET_ENTRY1 + BOOT_TABLE_ENTRY_SIZE * 0, BOOT_TABLE_ENTRY_SIZE);
    testResult = testResult + FlashTestSeg(BOOT_TABLE_OFFSET_ENTRY1 + BOOT_TABLE_ENTRY_SIZE * 1, BOOT_TABLE_ENTRY_SIZE);
    testResult = testResult + FlashTestSeg(BOOT_TABLE_OFFSET_ENTRY1 + BOOT_TABLE_ENTRY_SIZE * 2, BOOT_TABLE_ENTRY_SIZE);
    testResult = testResult + FlashTestSeg(BOOT_TABLE_OFFSET_ENTRY1 + BOOT_TABLE_ENTRY_SIZE * 3, BOOT_TABLE_ENTRY_SIZE);
    testResult = testResult + FlashTestSeg(BOOT_TABLE_OFFSET_ENTRY1 + BOOT_TABLE_ENTRY_SIZE * 4, BOOT_TABLE_ENTRY_SIZE);
    testResult = testResult + FlashTestSeg(BOOT_TABLE_OFFSET_ENTRY1 + BOOT_TABLE_ENTRY_SIZE * 5, BOOT_TABLE_ENTRY_SIZE);
    testResult = testResult + FlashTestSeg(BOOT_TABLE_OFFSET_ENTRY1 + BOOT_TABLE_ENTRY_SIZE * 6, BOOT_TABLE_ENTRY_SIZE);

    testResult = testResult + FlashTestSeg(BOOT_TABLE_OFFSET_INDEX, BSP_EBI_FLASH_SSECTOR_SIZE);
    testResult = testResult + FlashTestSeg(BOOT_TABLE_OFFSET_CNTR, BSP_EBI_FLASH_SSECTOR_SIZE);
    testResult = testResult + FlashTestSeg(BOOT_TABLE_OFFSET_CRC, BSP_EBI_FLASH_SSECTOR_SIZE);
    testResult = testResult + FlashTestSeg(BOOT_TABLE_OFFSET_TEST, BSP_EBI_FLASH_SSECTOR_SIZE);

    if (testResult == 11) // Test passed
    {
        debugLen = sprintf((char*)debugStr, "\nFlash test Passed");
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
        testResultEBI[1] = 1;
    }
    else
    {
        debugLen = sprintf((char*)debugStr, "\nFlash test Failed!");
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
        testResultEBI[1] = 0;
    }
}

uint8_t FlashTestSeg(uint32_t offset, uint32_t segsize)
{
    uint32_t i;
    DEVSTATUS status;

    // Transmit Status update
    debugLen = sprintf((char*)debugStr, "\nTesting external flash segment at offset %08x", offset);
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

    // Erase the flash
    if (TopBoot)
    {
        if ((offset >= 0X000000) && (offset <= 0x3E0000)) // lARGE segments
        {
            for (i = 0; i < segsize; i += BSP_EBI_FLASH_LSECTOR_SIZE)
            {
                status = lld_SectorEraseOp((uint8_t*)BOOT_TABLE_BASE, offset + i);
                if (status != DEV_NOT_BUSY)
                {
                    debugLen = sprintf((char*)debugStr, "\nERROR: lld_SectorEraseOp returned %d (offset = %d)", status, offset + i);
                    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
                }
            }
        }
        else if ((offset >= 0x3F0000) && (offset <= 0x3FE000)) // SMALL segments
        {
            for (i = 0; i < segsize; i += BSP_EBI_FLASH_SSECTOR_SIZE)
            {
                status = lld_SectorEraseOp((uint8_t*)BOOT_TABLE_BASE, offset + i);
                if (status != DEV_NOT_BUSY)
                {
                    debugLen = sprintf((char*)debugStr, "\nERROR: lld_SectorEraseOp returned %d (offset = %d)", status, offset + i);
                    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
                }
            }
        }
    }
    else if (BotBoot)
    {
        if ((offset >= 0X000000) && (offset <= 0x00E000)) // SMALL segments
        {
            for (i = 0; i < segsize; i += BSP_EBI_FLASH_SSECTOR_SIZE)
            {
                status = lld_SectorEraseOp((uint8_t*)BOOT_TABLE_BASE, offset + i);
                if (status != DEV_NOT_BUSY)
                {
                    debugLen = sprintf((char*)debugStr, "\nERROR: lld_SectorEraseOp returned %d (offset = %d)", status, offset + i);
                    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
                }
            }
        }
        else if ((offset >= 0x010000) && (offset <= 0x3F0000)) // LARGER segments
        {
            for (i = 0; i < segsize; i += BSP_EBI_FLASH_LSECTOR_SIZE)
            {
                status = lld_SectorEraseOp((uint8_t*)BOOT_TABLE_BASE, offset + i);
                if (status != DEV_NOT_BUSY)
                {
                    debugLen = sprintf((char*)debugStr, "\nERROR: lld_SectorEraseOp returned %d (offset = %d)", status, offset + i);
                    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
                }
            }
        }
    }

    // Write to FLASH
    int errcnt = 0;
    for (i = 0; i < segsize; i++)
    {
        status = lld_ProgramOp((uint8_t*)(BOOT_TABLE_BASE), (offset + i), testValue(i));
        if (status != DEV_NOT_BUSY)
        {
            errcnt++;
        }
    }
    if (errcnt != 0) // Output Error msg if there are any errors
    {
        debugLen = sprintf((char*)debugStr, "\nERROR: lld_ProgramOp failed %d times", errcnt);
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    }

    // Read external FLASH
    errcnt = 0;
    for (i = 0; i < segsize; i++)
    {
        if (*(uint8_t*)(BOOT_TABLE_BASE + offset + i) != testValue(i)) // Compare to written value
        {
            errcnt++;
        }
    }
    if (errcnt != 0)
    {
        debugLen = sprintf((char*)debugStr, "\nERROR: verify failed %d times", errcnt);
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
        return 0;
    }
    else
    {
        debugLen = sprintf((char*)debugStr, "\nSegment at offset 0x%08x passed test", offset);
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
        return 1;
    }
    return 0;
}

void TEST_SRAM(void)
{
    debugLen = sprintf((char*)debugStr, "\n\nRunning complete SRAM test:");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

    // Test Both SRAMs
    debugLen = sprintf((char*)debugStr, "\n\nTest with both SRAMs on (Assume both are on, check status):");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    PerformSRAMtest();

    // Test SRAM 1
    BSP_EBI_disableSRAM(bspEbiSram2);
    Sram2EnabledStatus = 0;
    debugLen = sprintf((char*)debugStr, "\n\nTest SRAM1:");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    PerformSRAMtest();
    Delay(5);

    // Test SRAM 2
    Sram2EnabledStatus = BSP_EBI_enableSRAM(bspEbiSram2);
    BSP_EBI_disableSRAM(bspEbiSram1);
    Sram1EnabledStatus = 0;
    debugLen = sprintf((char*)debugStr, "\n\nTest SRAM2:");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    PerformSRAMtest();
    Delay(5);

    // Sanity test with both SRAM off
    BSP_EBI_disableSRAM(bspEbiSram1);
    BSP_EBI_disableSRAM(bspEbiSram2);
    Sram1EnabledStatus = 0;
    Sram2EnabledStatus = 0;
    debugLen = sprintf((char*)debugStr, "\n\nRun test with both off:");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    PerformSRAMtest();
    Delay(5);

    // Test SRAM 2 again
    Sram2EnabledStatus = BSP_EBI_enableSRAM(bspEbiSram2);
    debugLen = sprintf((char*)debugStr, "\n\nTest SRAM2 again:");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    PerformSRAMtest();
    Delay(5);

    // Test SRAM 1 again
    BSP_EBI_disableSRAM(bspEbiSram2);
    Sram2EnabledStatus = 0;
    Sram1EnabledStatus = BSP_EBI_enableSRAM(bspEbiSram1);
    debugLen = sprintf((char*)debugStr, "\n\nTest SRAM1 again:");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    PerformSRAMtest();
    Delay(5);

    // Test BOTH SRAM again
    Sram2EnabledStatus = BSP_EBI_enableSRAM(bspEbiSram2);
    debugLen = sprintf((char*)debugStr, "\n\nTest both SRAMs:");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    PerformSRAMtest();
}

void PerformSRAMtest()
{
    uint32_t i = 0;
    uint32_t FailMark = BSP_EBI_SRAM_SIZE + 1;
    uint32_t Test1Fail_addres = BSP_EBI_SRAM_SIZE + 1;

    testResultEBI[2] = 1;

    debugLen = sprintf((char*)debugStr,
        "\nSRAM status:\nSRAM 1 latched: %d \nSRAM 2 latched: %d \nSRAM 1 On: %d \nSRAM 2 On: %d",
        BSP_SEE_isSramLatched(bspSeeSram1),
        BSP_SEE_isSramLatched(bspSeeSram2),
        Sram1EnabledStatus,
        Sram2EnabledStatus);
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

    // Runing First test: Write to address and then Read From Address. (if there is somthing wring with the address allocation this test
    // will not detect the fault)
    // This test only confirms the ability to write to and read from the SRAM.
    debugLen = sprintf((char*)debugStr, "\nPerforming SRAM Write and Read test");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    for (i = 0; i < BSP_EBI_SRAM_SIZE; i++)
    {
        *(uint8_t*)(BSP_EBI_SRAM1_BASE + i) = testValue(i);
        *(uint8_t*)(BSP_EBI_SRAM1_BASE + i + 1) = testValue(i + 1);
        if (*(uint8_t*)(BSP_EBI_SRAM1_BASE + i) != testValue(i))
        {
            if (Test1Fail_addres == (BSP_EBI_SRAM_SIZE + 1))
            {
                Test1Fail_addres = i;
                testResultEBI[2] = 0;
                break; // Break out of For loop
            }
        }
    }
    // Report Test 1 results
    if (testResultEBI[2] == 0)
    {
        debugLen = sprintf(
            (char*)debugStr, "\nSRAM write read test failed! Address offset where first error was encountered: %d", Test1Fail_addres);
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    }
    else
    {
        debugLen = sprintf((char*)debugStr, "\nSRAM write and read test was successful!");
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    }

    // The Second test writes to the entire SRAM and then only when all writing is complete does it start rading and comparing the vlaues
    // form the start. If a value was written in the wrong address this test will ditect the error
    debugLen = sprintf((char*)debugStr, "\nPerforming SRAM Write and Read test, with correct addressing");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    for (i = 0; i < BSP_EBI_SRAM_SIZE; i++)
    {
        *(uint8_t*)(BSP_EBI_SRAM1_BASE + i) = testValue(i);
    }

    for (i = 0; i < BSP_EBI_SRAM_SIZE; i++)
    {
        if (*(uint8_t*)(BSP_EBI_SRAM1_BASE + i) != testValue(i))
        {
            testResultEBI[2] = 0;
            if (FailMark == (BSP_EBI_SRAM_SIZE + 1))
            {
                FailMark = i;
            }
            break;
        }
    }
    // Report Test 2 results
    if (testResultEBI[2] == 0)
    {
        debugLen = sprintf((char*)debugStr, "\nSRAM test failed! Address where first wrong value was read: %d", FailMark);
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    }
    else
    {
        debugLen = sprintf((char*)debugStr, "\nSRAM addressing test  successful!");
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
    }
}

/******************************* EBI ***************************************/ /**
  *
  * @author Pieter J. Botma
  * @date   04/04/2013
  *
  * This functions test the EBI by writing and reading to the different external
  * memory modules and then displaying the results over the debug UART.
  *
  ******************************************************************************/
void TEST_EBI(void)
{
    debugLen = sprintf((char*)debugStr, "\n\n******************** TESTING EBI BUS *************************");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
//*** EEPROM ***//
#if defined(Base)
    TEST_EEPROM_COMPLETE();
#elif defined(Debug)
    TEST_EEPROM_COMPLETE();
#elif defined(Release)
    debugLen = sprintf((char*)debugStr, "\n\nEEPROM cannot be tested in Release build");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
#endif

//*** Flash test ***//
#if defined(Base)
    TEST_FLASH_COMPLETE();
#elif defined(Debug) || defined(Release)
    TEST_FLASH();
#endif

//*** SRAM Test ***//
#if defined(Base) || defined(Debug)
    TEST_SRAM();
#elif defined(Release)
    debugLen = sprintf((char*)debugStr, "\n\nSRAM cannot be tested in Release build");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
#endif
    debugLen = sprintf((char*)debugStr, "\n\n****************** EBI BUS TEST COMPLETE ***********************");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
}

/******************************* I2C ***************************************/ /**
  *
  * @author Pieter J. Botma
  * @date   04/04/2013
  *
  * This function tests the master I2C functionality by writing and reading to an
  * external I2C module (ST M24C64) connected to the main system I2C channel.
  *
  ******************************************************************************/
void TEST_I2C(void)
{
    uint8_t txBuffer[5] = {0x00, 0x00, 0xA1, 0xA2, 0xA3}; // write commands = address1, addres2, data1, data2, data3
    uint8_t rxBuffer[5] = {0};                            // read data
    // I2C Master Test (Used with ST M24C64 EEPROM)
    debugLen = sprintf((char*)debugStr, "I2C Test (External EEPROM write and read):\n");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

    // BSP_I2C_masterTX(BSP_I2C_SYS, 0xA0, bspI2cWrite, txBuffer, 5, rxBuffer, 0);
    BSP_I2C_masterTX(BSP_I2C_SUB, 0xA0, bspI2cWrite, txBuffer, 5, rxBuffer, 0);
    Delay(100);
    // BSP_I2C_masterTX(BSP_I2C_SYS, 0xA0, bspI2cWriteRead, txBuffer, 2, rxBuffer, 3);
    BSP_I2C_masterTX(BSP_I2C_SUB, 0xA0, bspI2cWriteRead, txBuffer, 2, rxBuffer, 3);

    // Display results read from EEPROM (should display 0xA1, 0xA2, 0xA3)
    BSP_UART_txByte(BSP_UART_DEBUG, rxBuffer[0]);
    BSP_UART_txByte(BSP_UART_DEBUG, rxBuffer[1]);
    BSP_UART_txByte(BSP_UART_DEBUG, rxBuffer[2]);
}

/***************************** MICROSD *************************************/ /**
  *
  * @author Pieter J. Botma
  * @date   04/04/2013
  *
  * This functions tests the microSD functionality by detecting the SD card,
  * initialising the file system, writing a test buffer and reading it again.
  *
  * @note
  * 	SD card must be formated as FAT32.
  * @note
  *   Refer to Energy Micro application note AN0030.
  *
  *******************************************************************************/

void TEST_Wdg(bool Internal, bool External)
{
    if (Internal)
    {
        debugLen = sprintf((char*)debugStr, "\nTesting Internal Watchdog (4.097s timeout)");
    }
    else if (External)
    {
        debugLen = sprintf((char*)debugStr, "\nTesting External Watchdog (0.9-2.5s (1.6s TYP) timeout)");
    }

    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

    Delay(2000); // delay for 2s to print out to uart

    int time_counter = 0;
    BSP_WDG_Init(Internal, External); // Init the watchdogs
    if (External)
    {
        BSP_WDG_ToggleExt();
        Delay(1);
        BSP_WDG_ToggleExt();
        Delay(1);
        BSP_WDG_ToggleExt();
    }

    // Print out the time until the watchdog triggers a reset

    time_t current_time = time(NULL); // Obtain current time as seconds elapsed since Epoch

    if (current_time == (time_t)-1)
    {
        // error
    }
    while (time_counter < 100) // loop for 5 seconds (watchdog must trigger before 5 seconds with default settings)
    {
        debugLen = sprintf((char*)debugStr, "\nRuntime: %d ms", (int)msTicks);
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

        Delay(50); // delay for 50ms
        time_counter++;
    }
}
