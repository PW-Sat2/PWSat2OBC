#include <stdio.h>
#include <string.h>
#include <chrono>
#include <em_burtc.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_emu.h>
#include <em_rmu.h>
#include <em_usart.h>
#include "config.hpp"
#include "mcu/io_map.h"
#include "system.h"

using namespace std::chrono;

milliseconds elapsed_time = 0ms;

void BURTC_IRQHandler(void)
{
    BURTC_IntClear(BURTC_IntGet());
}

void ConfigureBurtc()
{
    static constexpr uint32_t InterruptPriority = 6;

    CMU_ClockEnable(cmuClock_CORELE, true);
    CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);
    RMU_ResetControl(rmuResetBU, rmuResetModeClear);

    BURTC_Init_TypeDef burtcInit = BURTC_INIT_DEFAULT;

    burtcInit.clkSel = burtcClkSelLFRCO;
    burtcInit.mode = burtcModeEM3;
    burtcInit.clkDiv = Config::PrescalerDivider;
    burtcInit.lowPowerComp = 0;
    BURTC_Init(&burtcInit);

    BURTC_IntClear(BURTC_IEN_COMP0);

    NVIC_SetPriority(BURTC_IRQn, InterruptPriority);

    BURTC_IntEnable(BURTC_IEN_COMP0);

    NVIC_ClearPendingIRQ(BURTC_IRQn);
    NVIC_EnableIRQ(BURTC_IRQn);
}

void ArmBurtc()
{
    BURTC_CompareSet(0, BURTC_CounterGet() + Config::BuRTCCompareValue);
}

milliseconds GetTime()
{
    return elapsed_time;
}

void UpdateTime() 
{
    elapsed_time += Config::TickLength;
}