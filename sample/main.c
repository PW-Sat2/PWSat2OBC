/***************************************************************************/ /**
  * @file
  * @brief   CubeComputer Test Program Main.
  * @author  Pieter Botma
  * @version 2.0
  ******************************************************************************/
#include "includes.h"                                                         //< Include all required header files
#define DATASCRUB_PIECESIZE 4   //(32 * 1024) ///< Data piece size scrubed when performing BSP_SEE_checkMemory().
#define LATCHUP_RETRY_COUNT 3   ///< Amount of latchup recovery retrys to perform.
uint8_t Sram1EnabledStatus = 2; ///< SRAM 1 on or off status flag.
uint8_t Sram2EnabledStatus = 2; ///< SRAM 2 on or off status flag.

/***************************************************************************/ /**
  * @brief  Main function
  * Main is called from _program_start, see assembly startup file
  ******************************************************************************/
int main(void)
{
#if defined(Debug)
    SCB->VTOR = 0x80000; // Set new vector table when built in debug mode
#endif
    CHIP_Init(); // Initialise chip
    BSP_Clocks_Init();
    BSP_DMA_Init();
    BSP_WDG_Init(false, false);
    BSP_UNIX_Init();
    BSP_ADC_Init();
#if defined(Base)   // In these Modes the Bootloader will not always be there to enable the SRAMs
    BSP_EBI_Init(); // Initiate the EBI
    Delay(5);
    Sram1EnabledStatus = BSP_EBI_enableSRAM(bspEbiSram1); // Enable SRAM 1
    Sram2EnabledStatus = BSP_EBI_enableSRAM(bspEbiSram2); // Enable SRAM 2
#endif
    COMMS_Init();                                           // Enable communication (UART, I2C and CAN)
    BSP_SEE_Init(DATASCRUB_PIECESIZE, LATCHUP_RETRY_COUNT); // Enable data scrubbing

    BACKGROUND_getResetCause();
    BACKGROUND_displayResetCause();     // Print out the reset cause over UART
    BACKGROUND_displayWelcomeMessage(); // Print out the welcome message over UART

    uint32_t tick, tock; // Control loop timing variables (One control loop is performed every second in this example)
    while (1)            // main loop
    {
        tock = time(NULL);
        /*
         * Add code here...
         */
        BSP_SEE_checkMemory(); // Perform data scrubbing

        do
        { // wait until end of control loop
            //			EMU_EnterEM1();
            COMMS_processTCMD();
            tick = time(NULL);
        } while (difftime(tick, tock) < 1); // sets the number of seconds for a control loop
    }
}
/***************************************************************************/ /**
  * @brief
  *   This function is required by the FAT file system in order to provide
  *   timestamps for created files. Since this example does not include a
  *   reliable clock we hardcode a value here.
  *
  *   Refer to drivers/fatfs/doc/en/fattime.html for the format of this DWORD.
  * @return
  *    A DWORD containing the current time and date as a packed data structure.
  ******************************************************************************/
DWORD get_fattime(void)
{
    time_t currentTime = time(NULL);
    struct tm calendar = *localtime(&currentTime);
    /* convert calendar to DWORD */
    return (28 << 25) | (2 << 21) | (1 << 16);
}
