/***************************************************************************/ /**
  * @file	bsp_unix.c
  * @brief	BSP UNIX source file.
  *
  * This file contains all the implementations for the functions defined in \em
  * bsp_unix.h.
  * @author	Pieter J. Botma
  * @date	16/05/2014
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
#include "bsp_unix.h"

/***************************************************************************/ /**
  * @addtogroup BSP_Library
  * @brief Board Support Package (<b>BSP</b>) Driver Library for CubeComputer.
  * @{
  ******************************************************************************/

/***************************************************************************/ /**
  * @addtogroup UNIX
  * @brief API for CubeComputer's Unix counter.
  * @{
  ******************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

#define BURTC_PRESCALING 128
#define LFXO_FREQUENCY 32768
#define COUNTS_PER_SEC (LFXO_FREQUENCY / BURTC_PRESCALING)

void initBackupPowerDomain(void);
void initBackupRTC(void);

/** @endcond */

static time_t unixEpoch = 0; ///< Unix counter start time.

/***************************************************************************/ /**
  * @author Pieter J. Botma
  * @date   16/05/2014
  *
  * This function implements the time() function defined in the <time.h> library.
  * It returns the current Unix time. The time is based on the epoch and counter
  * value.
  *
  * @param timer
  *   If not a null pointer, time is copied to this.
  *
  * @return
  *   Current system time. Should, but does not, return -1 if system time is
  *   not available.
  *
  *****************************************************************************/
time_t time(time_t* timer)
{
    time_t t;

    /* Add the time offset */
    t = unixEpoch;

    /* Add the number of seconds for BURTC */

    t += (BURTC_CounterGet() / COUNTS_PER_SEC);

    /* Copy system time to timer if not NULL*/
    if (!timer)
    {
        timer = &t;
    }

    return t;
}

/***************************************************************************/ /**
  * @author Pieter J. Botma
  * @date   16/05/2014
  *
  * This function initializes the Unix time module. The Unix epoch is set to the
  * specified epoch calendar. The Unix counter (based on the Real-Time-Clock) is
  * also initialized.
  *
  * @note
  *   If a brown out on the backup power was detected, the RTC is initialized
  *   regardless of any other cause. If not, the RTC is initialized if a reset
  *   cause occurred other than a system request, lockup or watchdog reset.
  ******************************************************************************/
void BSP_UNIX_Init()
{
    struct tm defaultCalendar = DEFAULT_UNIX_START_DATE;
    uint32_t resetcause = RMU->RSTCAUSE;
//    initBackupPowerDomain();

#if defined(CubeCompV3)
    if (!(resetcause & RMU_RSTCAUSE_BUBODVDDDREG) &&
        ((resetcause & RMU_RSTCAUSE_SYSREQRST) || (resetcause & RMU_RSTCAUSE_LOCKUPRST))) // If the reset cause was not Backup Brown-Out
                                                                                          // Detector on BUVIN, but caused by a system
                                                                                          // request or a lockup
    {
        unixEpoch = BURTC_RetRegGet(0); // Read the unixEpoch from the Backup RTC Retention register
        return;
    }
    else
    {
        initBackupRTC();
        BSP_UNIX_setEpochCalendar(&defaultCalendar);
    }
#endif

#if defined(CubeCompV3B)
    // if ( !(resetcause & RMU_RSTCAUSE_BUBODBUVIN) && ( (resetcause & RMU_RSTCAUSE_SYSREQRST) || (resetcause & RMU_RSTCAUSE_LOCKUPRST) ) )
    // //If the reset cause was not Backup Brown-Out Detector on BUVIN, but caused by a system request or a lockup (use if backup power
    // supply is available)
    if (!(resetcause & RMU_RSTCAUSE_BUBODVDDDREG) &&
        ((resetcause & RMU_RSTCAUSE_SYSREQRST) || (resetcause & RMU_RSTCAUSE_LOCKUPRST))) // If the reset cause was not Backup Brown-Out
                                                                                          // Detector on BUVIN, but caused by a system
                                                                                          // request or a lockup
    {
        unixEpoch = BURTC_RetRegGet(0); // Read the unixEpoch from the Backup RTC Retention register
                                        //        return;
    }
    else
    {
        //        initBackupRTC();
        //        BSP_UNIX_setEpochCalendar(&defaultCalendar);
    }
#endif

    initBackupRTC();
    BSP_UNIX_setEpochCalendar(&defaultCalendar);
}

/***************************************************************************/ /**
  * @author Pieter J. Botma
  * @date   16/05/2014
  *
  * This function sets the Unix epoch to the specified epoch calendar and
  * restarts the Unix counter.
  *
  * @param[in] epochCalender
  *   Calendar struct which is converted to unix time and used as new epoch.
  *
  ******************************************************************************/
void BSP_UNIX_setEpochCalendar(struct tm* epochCalendar)
{
    unixEpoch = mktime(epochCalendar);

    BURTC_RetRegSet(0, unixEpoch);
    BURTC_CounterReset();
}

/***************************************************************************/ /**
  * @author Pieter J. Botma
  * @date   16/05/2014
  *
  * This function sets the Unix epoch to the specified epoch and restarts the
  * Unix counter.
  *
  * @param[in] epoch
  *   Unix time of the new epoch.
  *
  ******************************************************************************/
void BSP_UNIX_setEpoch(time_t epoch)
{
    unixEpoch = epoch;

    BURTC_RetRegSet(0, unixEpoch);
    BURTC_CounterReset();
}

/***************************************************************************/ /**
  * @brief Get the epoch offset
  *
  * This function gets the current Unix epoch.
  *
  * @return
  *   Unix time of the current epoch.
  *
  ******************************************************************************/
time_t BSP_UNIX_getEpoch(void)
{
    return unixEpoch;
}

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

void initBackupPowerDomain(void)
{
    // Assign default TypeDefs and then setup EM4 configuration structure
    EMU_EM4Init_TypeDef em4Init = EMU_EM4INIT_DEFAULT;
    em4Init.lockConfig = true;     // Lock configuration after it's been set
    em4Init.osc = emuEM4Osc_LFRCO; // Use LFXO oscillator
    em4Init.buRtcWakeup = false;   // Do not wake up on BURTC interrupt
    em4Init.vreg = true;           // Enable VREG

    // Assign default TypeDefs and then setup Backup Power Domain configuration structure
    EMU_BUPDInit_TypeDef bupdInit = EMU_BUPDINIT_DEFAULT;
    bupdInit.probe = emuProbe_Disable;      // Do not enable voltage probe
    bupdInit.bodCal = false;                // Disable BOD calibration mode
    bupdInit.statusPinEnable = false;       // Disable BU_STAT pin for backup mode indication
    bupdInit.resistor = emuRes_Res0;        // Main power and backup power connected with RES0 series resistance
    bupdInit.voutStrong = false;            // Don't enable Strong output switch
    bupdInit.voutMed = false;               // Don't enable medium output switch
    bupdInit.voutWeak = false;              // Don't enable weak output switch
    bupdInit.inactivePower = emuPower_None; // No connection between main and backup power (inactive mode) (When not in Backup Mode)
    bupdInit.activePower = emuPower_None;   // No connection between main and backup power (active mode)	(When in Backup Mode)
    bupdInit.enable =
        true; // Enable backup power domain,and release reset, enable BU_VIN pin [enter on BOD, enable BU_VIN pin, release BU reset]

    EMU_EM4Lock(false); // Unlock configuration

    EMU_BUPDInit(&bupdInit); // Configure Backup Power Domain settings

#if defined(CubeCompV3B) // BU_VIN pin only available on CubeComputer V3B. User can access this pin on the Piggyback header (P600) pin 27. A
                         // suitable power source must be connected to pin to supply power to Backup power domain.
    EMU_BUPinEnable(true); // Enable BU_VIN support (BUVINPEN in EMU_ROUTE is cleared)
#endif
#if defined(CubeCompV3)     // BU_Vin pin is not available on CubeComputer V3A. The Backup power domain is only powered by the normal VDDReg
                            // received by the MCU.
    EMU_BUPinEnable(false); // Disable BU_VIN support (BUVINPEN in EMU_ROUTE is cleared)
#endif
    EMU_EM4Init(&em4Init); // Update EMU module with Energy Mode 4 configuration.

    EMU_BUReady(); // Halts until backup power functionality is ready

    RMU_ResetControl(rmuResetBU, rmuResetModeClear); // Enable backup power domain,and release reset

    EMU_EM4Lock(true); // Lock configuration
}

void initBackupRTC(void)
{
    CMU_ClockEnable(cmuClock_HFLE, true);
    CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
    RMU_ResetControl(rmuResetBU, rmuResetModeClear);
    BURTC_Init_TypeDef burtcInit = BURTC_INIT_DEFAULT;

    //    burtcInit.enable = true;             // Enable BURTC after initialization (Starts counter)
    burtcInit.mode = burtcModeEM4; // Configure energy mode operation: burtcModeEM4: Enable and start BURTC counter in EM0 to EM4
    //    burtcInit.debugRun = false;          // If true, counter will keep running under debug halt, flase so counter will stop under
    //    debug halt
    burtcInit.clkSel = burtcClkSelLFXO; // Select clock source: burtcClckSellFXO -> Low frequency crystal oscillator
    burtcInit.clkDiv = burtcClkDiv_128; // Clock divider: burtcClkDiv_128 - > 128
    //    burtcInit.timeStamp = false;         // In this case disable time stamp on entering backup power domain
    //    burtcInit.compare0Top = false;       // Set if Compare Value 0 is top value (counter restart)
    //    burtcInit.lowPowerMode = burtcLPDisable; // BURTC low power mode, burtcLPDisable -> Low power mode is disabled

    BURTC_Unlock();
    BURTC_Init(&burtcInit);
    //    BURTC_Lock();
}

/** @endcond */

/** @} (end addtogroup UNIX) */
/** @} (end addtogroup BSP_Library) */
