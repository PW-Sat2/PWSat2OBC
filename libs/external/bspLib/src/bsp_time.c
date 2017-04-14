/*
 * bsp_time.c
 *
 *  Created on: 11 Sep 2015
 *      Author: cjgroenewald
 */
#include "bsp_time.h"
volatile uint32_t delayTicks = 0; ///< Runtime counter used for calculating delays.
void BSP_Clocks_Init(void)
{
    // initialise oscillators
    //	CMU_OscillatorEnable(cmuOsc_HFXO, true, true); 	//< Enable the High frequency Crystal oscillator(external) and wait for the
    // oscillator start-up time to timeout before returning
    CMU_OscillatorEnable(cmuOsc_LFXO, true, true); //< Enable the Low frequency Crystal oscillator(external) and wait for the
    //    oscillator
    // start-up time to timeout before returning
    CMU_OscillatorEnable(cmuOsc_LFRCO, false, false);

    // Select reference clock/oscillator used for a clock branch
    CMU_ClockSelectSet(
        cmuClock_HF, cmuSelect_HFRCO); // Set High frequency clock branch reference to the High frequency Crystal oscillator(external).
    CMU_ClockSelectSet(
        cmuClock_LFA, cmuSelect_LFXO); // Set Low frequency A clock branch reference to the High frequency Crystal oscillator(external).

    CMU_ClockEnable(cmuClock_HFLE, true); // Enable the Low energy clocking module clock

    //    CMU_OscillatorEnable(cmuOsc_HFRCO, false, false); // Disable the High Frequency RC oscillator
    //    CMU_OscillatorEnable(cmuOsc_LFRCO, true, true); // Disable the Low Frequency RC oscillator

    if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) // Setup SysTick Timer for 1 msec interrupts
    {
        while (1)
            ;
    }
    msTicks = 0;
}
/**************************************************************************/ /**
  * @brief SysTick_Handler
  * Interrupt Service Routine for system tick counter.
  *****************************************************************************/
void SysTick_Handler(void)
{
    msTicks++;
    delayTicks++;
    // BSP_WDG_ToggleExt();	//Enable this function to toggle the external watchdog every ms
}
/***************************************************************************/ /**
  * @brief Delays number of msTick Systicks (typically 1 ms)
  * @param dlyTicks Number of ticks to delay
  ******************************************************************************/
void Delay(uint32_t delay)
{
    delayTicks = 0;
    while (delayTicks < delay)
        ;
}
