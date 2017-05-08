
/***************************************************************************/ /**
  * @file	bsp_ebi.c
  * @brief	BSP EBI source file.
  *
  * This file contains all the implementations for the functions defined in \em
  * bsp_ebi.h.
  * @author	Pieter J. Botma
  * @date	28/05/2012
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
#include "bsp_ebi.h"

void setConfigureTopBoot();
void setConfigBotBoot();

#define testValue(addr) ((uint8_t)((addr * 7) % 255))

uint32_t BOOT_TABLE_OFFSET_INDEX;  ///< Offset for boot index
uint32_t BOOT_TABLE_OFFSET_CNTR;   ///< Offset for boot counter
uint32_t BOOT_TABLE_OFFSET_CRC;    ///< Offset for current CRC
uint32_t BOOT_TABLE_OFFSET_TEST;   ///< Offset for flash test space
uint32_t BOOT_TABLE_OFFSET_ENTRY1; ///< Offset for base of first table entry

uint8_t TopBoot = 0;
uint8_t BotBoot = 0;

/***************************************************************************/ /**
  * @addtogroup BSP_Library
  * @brief Board Support Package (<b>BSP</b>) Driver Library for CubeComputer.
  * @{
  ******************************************************************************/

/***************************************************************************/ /**
  * @addtogroup EBI
  * @brief API for CubeComputer's external bus interface.
  * @{
  ******************************************************************************/

/***************************************************************************/ /**
  * @author Pieter J. Botma
  * @date   28/05/2012
  *
  * This function initialise the CubeComputer's External Bus Interface (EBI)
  * which allows the use of CubeComputer's external memory modules (EEPROM, Flash
  * and SRAM).
  *
  ******************************************************************************/
void BSP_EBI_Init(void)
{
    unsigned int FlashID = 0;
    unsigned int FlashBootConfig = 0;
    // Enable clocks
    CMU_ClockEnable(cmuClock_EBI, true);
    CMU_ClockEnable(cmuClock_GPIO, true);

    // Configure Power to SRAM and IO buffers
    GPIO_PinModeSet(BSP_EBI_SRAM_BUFPORT, BSP_EBI_SRAM1_BUFPIN, gpioModePushPull, 1); // nOE on BUFFERS for SRAM1
    GPIO_PinModeSet(BSP_EBI_SRAM_BUFPORT, BSP_EBI_SRAM2_BUFPIN, gpioModePushPull, 1); // nOE on BUFFERS for SRAM2
    GPIO_PinModeSet(BSP_EBI_SRAM_POWPORT, BSP_EBI_SRAM1_POWPIN, gpioModePushPull, 0); // POW_SRAM1
    GPIO_PinModeSet(BSP_EBI_SRAM_POWPORT, BSP_EBI_SRAM2_POWPIN, gpioModePushPull, 0); // POW_SRAM2

#if defined(CubeCompV3) || defined(CubeCompV3B)
    // ECC  Control Pins
    GPIO_PinModeSet(gpioPortE, 2, gpioModePushPull, 0); // This should be 1 for disable
    GPIO_PinModeSet(gpioPortE, 3, gpioModePushPull, 0); // This should be 1 for disable
    // ECC  Error Pins
    GPIO_PinModeSet(gpioPortB, 0, gpioModeInput, 1);
    GPIO_PinModeSet(gpioPortB, 2, gpioModeInput, 1);
#endif

    // Configure GPIO pins as push pull
    // EBI AD9..15
    GPIO_PinModeSet(gpioPortA, 0, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortA, 1, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortA, 2, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortA, 3, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortA, 4, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortA, 5, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortA, 6, gpioModePushPull, 0);

    // EBI AD8
    GPIO_PinModeSet(gpioPortA, 15, gpioModePushPull, 0);

    // EBI nCS0..nCS3
    GPIO_PinModeSet(gpioPortD, 9, gpioModePushPull, 1);
    GPIO_PinModeSet(gpioPortD, 10, gpioModePushPull, 1);
    GPIO_PinModeSet(gpioPortD, 11, gpioModePushPull, 1);
    GPIO_PinModeSet(gpioPortD, 12, gpioModePushPull, 1);

    // EBI AD0..7
    GPIO_PinModeSet(gpioPortE, 8, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 9, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 10, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 11, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 12, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 13, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 14, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 15, gpioModePushPull, 0);

    // EBI ALEN/nWen/nRen
    GPIO_PinModeSet(gpioPortF, 3, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortF, 4, gpioModePushPull, 1);
    GPIO_PinModeSet(gpioPortF, 5, gpioModePushPull, 1);

    // EBI register initializations

    EBI_Init_TypeDef init = EBI_INIT_DEFAULT;

    init.mode = ebiModeD8A24ALE;

    init.banks = EBI_BANK0 | EBI_BANK1 | EBI_BANK2 | EBI_BANK3;
    init.csLines = EBI_CS0 | EBI_CS1 | EBI_CS2 | EBI_CS3;

    init.alePolarity = ebiActiveHigh;

    //    uint16_t factor = 1;

    // Address Setup and hold time
    init.addrHoldCycles = 3;
    init.addrSetupCycles = 3;

    // Read cycle times
    init.readStrobeCycles = 10;
    init.readHoldCycles = 3;
    init.readSetupCycles = 3;

    // Write cycle times
    init.writeStrobeCycles = 10;
    init.writeHoldCycles = 3;
    init.writeSetupCycles = 3;

    EBI_Init(&init);

    BSP_EBI_disableSRAM(bspEbiSram1);
    BSP_EBI_disableSRAM(bspEbiSram2);

    // Initialize comparators for SRAM
    BSP_ACMP_Init(BSP_ACMP_SRAM1);
    BSP_ACMP_Init(BSP_ACMP_SRAM2);

#if defined(CubeCompV3) || defined(CubeCompV3B)
    // Configure interupt on EDAC error pins
    GPIO_IntConfig(gpioPortB, 0, false, true, false);
    GPIO_IntConfig(gpioPortB, 1, false, true, false);
    GPIO_IntConfig(gpioPortB, 2, false, true, false);
#endif

    // Read the Flash configuration:
    FlashID = lld_GetDeviceId((uint8_t*)0x84000000);
    FlashID = FlashID & 0x00FF0000;
    FlashBootConfig = lld_ReadCfiWord((uint8_t*)0x84000000, 0x9E);

    switch (FlashID)
    {
        case 0x00F60000:                       // S29AL032D device Model 03
            if (FlashBootConfig == 0x00000003) // Model 03 Top Boot Device
            {
                setConfigureTopBoot();
                TopBoot = 1;
                BotBoot = 0;
            }
            else if (FlashBootConfig == 0x00000002) // Model 04 Bottom Boot Device
            {
                setConfigBotBoot();
                TopBoot = 0;
                BotBoot = 1;
            }
            break;

        case 0x00F90000:                       // S29AL032D device Model 04
            if (FlashBootConfig == 0x00000003) // Model 03 Top Boot Device
            {
                setConfigureTopBoot();
                TopBoot = 1;
                BotBoot = 0;
            }
            else if (FlashBootConfig == 0x00000002) // Model 04 Bottom Boot Device
            {
                setConfigBotBoot();
                TopBoot = 0;
                BotBoot = 1;
            }
            break;

        case 0x00000000:                       // S29JL032H device or S29JL032J device Model 01 or 02
            if (FlashBootConfig == 0x00000003) // 03 = Top Boot device
            {
                setConfigureTopBoot();
                TopBoot = 1;
                BotBoot = 0;
            }
            else if (FlashBootConfig == 0x00000002) // 02 Bottom Boot Device
            {
                setConfigBotBoot();
                TopBoot = 0;
                BotBoot = 1;
            }
            break;

        case 0x00010000:                       // S29JL032H device or S29JL032J device Model 01 or 02
            if (FlashBootConfig == 0x00000003) // 03 = Top Boot device
            {
                setConfigureTopBoot();
                TopBoot = 1;
                BotBoot = 0;
            }
            else if (FlashBootConfig == 0x00000002) // 02 Bottom Boot Device
            {
                setConfigBotBoot();
                TopBoot = 0;
                BotBoot = 1;
            }
            break;

        case 0x00560000:                       // S29JL032H device or S29JL032J device Model 21 or 22
            if (FlashBootConfig == 0x00000003) // 03 = Top Boot device
            {
                setConfigureTopBoot();
                TopBoot = 1;
                BotBoot = 0;
            }
            else if (FlashBootConfig == 0x00000002) // 02 Bottom Boot Device
            {
                setConfigBotBoot();
                TopBoot = 0;
                BotBoot = 1;
            }
            break;

        case 0x00550000:                       // S29JL032H device or S29JL032J device Model 21 or 22
            if (FlashBootConfig == 0x00000003) // 03 = Top Boot device
            {
                setConfigureTopBoot();
                TopBoot = 1;
                BotBoot = 0;
            }
            else if (FlashBootConfig == 0x00000002) // 02 Bottom Boot Device
            {
                setConfigBotBoot();
                TopBoot = 0;
                BotBoot = 1;
            }
            break;

        case 0x00530000:                       // S29JL032H device or S29JL032J device
            if (FlashBootConfig == 0x00000003) // 03 = Top Boot device
            {
                setConfigureTopBoot();
                TopBoot = 1;
                BotBoot = 0;
            }
            else if (FlashBootConfig == 0x00000002) // 02 Bottom Boot Device
            {
                setConfigBotBoot();
                TopBoot = 0;
                BotBoot = 1;
            }
            break;
        case 0x00500000:                       // S29JL032H device or S29JL032J device Model 31 or 32
            if (FlashBootConfig == 0x00000003) // 03 = Top Boot device
            {
                setConfigureTopBoot();
                TopBoot = 1;
                BotBoot = 0;
            }
            else if (FlashBootConfig == 0x00000002) // 02 Bottom Boot Device
            {
                setConfigBotBoot();
                TopBoot = 0;
                BotBoot = 1;
            }
            break;

        case 0x005F0000:                       // S29JL032H device or S29JL032J device Model 41 or 42
            if (FlashBootConfig == 0x00000003) // 03 = Top Boot device
            {
                setConfigureTopBoot();
                TopBoot = 1;
                BotBoot = 0;
            }
            else if (FlashBootConfig == 0x00000002) // 02 Bottom Boot Device
            {
                setConfigBotBoot();
                TopBoot = 0;
                BotBoot = 1;
            }
            break;

        case 0x005C0000:                       // S29JL032H device or S29JL032J device Model 41 or 42
            if (FlashBootConfig == 0x00000003) // 03 = Top Boot device
            {
                setConfigureTopBoot();
                TopBoot = 1;
                BotBoot = 0;
            }
            else if (FlashBootConfig == 0x00000002) // 02 Bottom Boot Device
            {
                setConfigBotBoot();
                TopBoot = 0;
                BotBoot = 1;
            }
            break;

        default:
            break;
    }
}

void setConfigureTopBoot()
{
    BOOT_TABLE_OFFSET_INDEX = 0x003F0000;  ///< Offset for boot index
    BOOT_TABLE_OFFSET_CNTR = 0x003F2000;   ///< Offset for boot counter
    BOOT_TABLE_OFFSET_CRC = 0x003F4000;    ///< Offset for current CRC
    BOOT_TABLE_OFFSET_TEST = 0x003FE000;   ///< Offset for flash test space
    BOOT_TABLE_OFFSET_ENTRY1 = 0x00000000; ///< Offset for base of first table entry
}

void setConfigBotBoot()
{
    BOOT_TABLE_OFFSET_INDEX = 0x00000000;  ///< Offset for boot index
    BOOT_TABLE_OFFSET_CNTR = 0x00002000;   ///< Offset for boot counter
    BOOT_TABLE_OFFSET_CRC = 0x00004000;    ///< Offset for current crc
    BOOT_TABLE_OFFSET_TEST = 0x0000E000;   ///< Offset for flash test space
    BOOT_TABLE_OFFSET_ENTRY1 = 0x00080000; ///< Offset for base of first table entry
}

/***************************************************************************/ /**
  * @author Pieter J. Botma
  * @date   17/07/2013
  *
  * This function tries to enable the specified SRAM module on the EBI. A 1ms
  * blanket period is given to allow in-rush-current to settle down before
  * checking for a latchup current.
  *
  * @param[in] module
  *   SRAM module to be enabled.
  *
  * @return
  *   Whether the specified SRAM module could be enable or not.
  *
  ******************************************************************************/
uint8_t BSP_EBI_enableSRAM(BSP_EBI_SRAMSelect_TypeDef module)
{
    uint8_t result;
    uint8_t TestValue = 0xCC;
    uint8_t ReadValue = 0x00;

    NVIC_DisableIRQ(ACMP0_IRQn);      // Disable IRQ handler for the Analogue Comparator (Disable Latchup protection for startup)
    NVIC_ClearPendingIRQ(ACMP0_IRQn); // Clear interrupts on the IRQ handler for the analog Comparator

    switch (module)
    {
        /*Power switch to each SRAM: This switch has over-current protection and will shut off if the constant current condition persists
         * after 10ms.
         * The switch will remain off after a current limit fault until the ON pin is cycled */
        case bspEbiSram1:
            setSRAM1Latched(0);
            BSP_EBI_Sram1PowOn(); // Turn on switch to SRAM 1

            Delay(3); // Wait for 3ms for in-rush currents to pass.

            if (false && ACMP0->STATUS & ACMP_STATUS_ACMPOUT) // disable SRAM if a latch is detected (over current condition)
            {
                BSP_EBI_Sram1PowOff(); // Turn of the switch to SRAM 1
                result = 0;            // Result set to 0 as SRAM 1 could not be turned on successfully
                setSRAM1Latched(1);
            }
            else
            {
                BSP_EBI_Sram1BuffOn();
#if defined(CubeCompV3) || defined(CubeCompV3B)
                BSP_EBI_Sram1FPGAOn(); // Enable the use of SRAM 1 in the FPGA
#endif
                // SRAM1 is now connected and on. Test by writing and reading to the SRAMs. Note that this test will pass if SRAM2 is
                // already turned on even tough SRAM1 might not be working.
                *(uint8_t*)(BSP_EBI_SRAM1_BASE + 10) = TestValue; // Write test value to the first SRAM address
                Delay(1);
                *(uint8_t*)(BSP_EBI_SRAM1_BASE + 11) = TestValue + 1; // Write a value to nexxt address
                Delay(1);
                ReadValue = *(uint8_t*)(BSP_EBI_SRAM1_BASE + 10); // Read test value from the first address in SRAM
                if (ReadValue != TestValue)
                {
                    result = 0; // If the wrong value is read something went wrong and the SRAM was not successfully turned on and connected
                                // to the bus
                    BSP_EBI_disableSRAM(bspEbiSram1); // Disable SRAM
                }
                else
                {
                    result = 1; // The value read is the same as the value written. The SRAM is on and connected to the buss successfully.
                }
            }

            ACMP_IntClear(ACMP0, ACMP_IFC_EDGE); // clear interrupt generated from in-rush current
            ACMP_IntClear(ACMP1, ACMP_IFC_EDGE); // clear interrupt generated from in-rush current
            break;

        case bspEbiSram2:
            setSRAM2Latched(0);
            BSP_EBI_Sram2PowOn(); // Turn on the switch to SRAM 2

            Delay(3); // Wait for 3ms for in-rush currents to pass.

            if (false && ACMP1->STATUS & ACMP_STATUS_ACMPOUT) // Disable SRAM if latch is detected
            {
                BSP_EBI_Sram2PowOff(); // Turn of the switch to SRAM 2
                result = 0;            // Set result to 0 because SRAM 2 could not be turned on successfully
                setSRAM2Latched(1);
            }
            else
            {
                BSP_EBI_Sram2BuffOn();
#if defined(CubeCompV3) || defined(CubeCompV3B)
                BSP_EBI_Sram2FPGAOn(); // Enable the use of SRAM2 in the FPGA
#endif
                // SRAM2 is now connected and on. Test that we can write and read to the SRAMs. Note that this test will pass if the SRAM1
                // is already turned on.
                *(uint8_t*)(BSP_EBI_SRAM1_BASE + 10) = TestValue; // Write test value to the first SRAM address
                Delay(1);
                *(uint8_t*)(BSP_EBI_SRAM1_BASE + 11) = TestValue + 1; // Write a value to nexxt address
                Delay(1);
                ReadValue = *(uint8_t*)(BSP_EBI_SRAM1_BASE + 10);
                if (ReadValue != TestValue)
                {
                    result = 0; // If the wrong value is read something went wrong and the SRAM was not successfully turned on and connected
                                // to the bus
                    BSP_EBI_disableSRAM(bspEbiSram2); // Disable SRAM
                }
                else
                {
                    result = 1;
                }
            }

            ACMP_IntClear(ACMP1, ACMP_IFC_EDGE); // clear interrupt generated from in rush current
            ACMP_IntClear(ACMP0, ACMP_IFC_EDGE); // clear interrupt generated from in-rush current
            break;

        default:
            result = 0;
            break;
    }

    NVIC_ClearPendingIRQ(ACMP0_IRQn); // Clear interrupts on the IRQ handler for the analog Comparator
    NVIC_EnableIRQ(ACMP0_IRQn);       // Enable the IRQ handler for the analog Comparator

    return result;
}

/***************************************************************************/ /**
  * @author Pieter J. Botma
  * @date   17/07/2013
  *
  * This function disables the specified SRAM module from the EBI.
  *
  * @param[in] module
  *   SRAM module to be disabled.
  *
  ******************************************************************************/
void BSP_EBI_disableSRAM(BSP_EBI_SRAMSelect_TypeDef module)
{
    switch (module)
    {
        case bspEbiSram1:
#if defined(CubeCompV3) || defined(CubeCompV3B)
            BSP_EBI_Sram1FPGAOff();
#endif
            BSP_EBI_Sram1PowOff();
            BSP_EBI_Sram1BuffOff();
            setSRAM1Latched(0);
            break;

        case bspEbiSram2:
#if defined(CubeCompV3) || defined(CubeCompV3B)
            BSP_EBI_Sram2FPGAOff();
#endif
            BSP_EBI_Sram2PowOff();
            BSP_EBI_Sram2BuffOff();
            setSRAM2Latched(0);
            break;
    }
}

/***************************************************************************/ /**
  * @author Pieter J. Botma
  * @date   07/06/2012
  *
  * This function writes a data \b buffer of length, \b len, to the EEPROM,
  * starting at a specified \b offset.
  *
  * @param[in] offset
  *   The address offset the data buffer should be written to.
  * @param[in] buffer
  *   The pointer to the data buffer to be written to the EEPROM.
  * @param[in] len
  *   The length of the data buffer to be written to the EEPROM.
  *
  ******************************************************************************/
void BSP_EBI_progEEPROM(uint32_t offset, uint8_t* buffer, uint8_t len)
{
    volatile uint8_t* eepromAddr = (volatile uint8_t*)(BSP_EBI_EEPROM_BASE + offset);

    int i = 0;

    // Write data
    do
    {
        // Unlock commands
        *(volatile uint8_t*)(BSP_EBI_EEPROM_BASE + 0x5555) = 0xAA;
        *(volatile uint8_t*)(BSP_EBI_EEPROM_BASE + 0x2AAA) = 0x55;
        *(volatile uint8_t*)(BSP_EBI_EEPROM_BASE + 0x5555) = 0xA0;

        do
        {
            *(eepromAddr + i) = buffer[i];
            i++;
        } while ((i < len) && ((i % 64) != 0));

        // Poll write sequence completion
        while (((*(eepromAddr + i - 1)) & BSP_EBI_EEPROM_POLL_MASK) != (buffer[i - 1] & BSP_EBI_EEPROM_POLL_MASK))
            ;
    } while (i < len);
}

/** @} (end addtogroup EBI) */
/** @} (end addtogroup BSP_Library) */
