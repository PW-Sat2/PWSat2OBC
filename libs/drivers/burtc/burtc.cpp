#include "burtc.hpp"
#include "base/os.h"
#include "em_burtc.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_rmu.h"
#include "logger/logger.h"
#include "system.h"

using namespace devices::burtc;
using namespace std::chrono_literals;

static OSSemaphoreHandle burtcInterruptSemaphore;

void BURTC_IRQHandler(void)
{
    uint32_t irq = BURTC_IntGet();
    BURTC_IntClear(irq);
    BURTC_CompareSet(0, BURTC->COMP0 + Burtc::CompareValue);

    System::GiveSemaphoreISR(burtcInterruptSemaphore);
}

Burtc::Burtc(BurtcTickCallback& tickCallback) : _tickCallback(tickCallback), _timeDelta(0)
{
}

void Burtc::HandleTickTask(void* arg)
{
    Burtc* burtcObject = static_cast<Burtc*>(arg);

    while (1)
    {
        System::TakeSemaphore(burtcInterruptSemaphore, InfiniteTimeout);

        burtcObject->_tickCallback.Tick(burtcObject->_timeDelta);
    }
}

void Burtc::Initialize()
{
    burtcInterruptSemaphore = System::CreateBinarySemaphore();

    System::CreateTask(Burtc::HandleTickTask, "HandleTickTask", 512, this, TaskPriority::P1, NULL);

    ConfigureHardware();
}

std::chrono::milliseconds Burtc::CalculateCurrentTimeInterval()
{
    return std::chrono::milliseconds(1000 * CompareValue / BURTC_ClockFreqGet());
}

void Burtc::ConfigureHardware()
{
    CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);
    RMU_ResetControl(rmuResetBU, rmuResetModeClear);

    BURTC_Init_TypeDef burtcInit = BURTC_INIT_DEFAULT;

    burtcInit.clkSel = burtcClkSelLFRCO;
    burtcInit.mode = burtcModeEM4;
    burtcInit.clkDiv = PrescalerDivider;
    BURTC_Init(&burtcInit);

    _timeDelta = CalculateCurrentTimeInterval();

    BURTC_IntClear(BURTC_IEN_COMP0);

    NVIC_SetPriority(BURTC_IRQn, InterruptPriority);
    NVIC_ClearPendingIRQ(BURTC_IRQn);
    NVIC_EnableIRQ(BURTC_IRQn);

    BURTC_CompareSet(0, BURTC_CounterGet() + CompareValue);

    BURTC_IntEnable(BURTC_IEN_COMP0);
}
