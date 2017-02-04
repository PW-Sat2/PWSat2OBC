#include "burtc.hpp"
#include "em_burtc.h"
#include "em_cmu.h"
#include "em_rmu.h"
#include "logger/logger.h"

using namespace devices::burtc;
using namespace std::chrono_literals;

static OSSemaphoreHandle burtcInterruptSemaphore;

Burtc::Burtc(TimeAction& tickCallback)
    : _tickCallback(tickCallback),                         //
      _timeDelta(0),                                       //
      _task("HandleTickTask", this, Burtc::HandleTickTask) //
{
}

void Burtc::IRQHandler()
{
    std::uint32_t irq = BURTC_IntGet();
    BURTC_IntClear(irq);
    BURTC_CompareSet(0, BURTC->COMP0 + Burtc::CompareValue);

    System::GiveSemaphoreISR(burtcInterruptSemaphore);

    System::EndSwitchingISR();
}

void Burtc::HandleTickTask(Burtc* burtcObject)
{
    while (1)
    {
        if (OS_RESULT_FAILED(System::TakeSemaphore(burtcInterruptSemaphore, InfiniteTimeout)))
        {
            LOG(LOG_LEVEL_ERROR, "Unable to take burtc interrupt semaphore.");
        }

        burtcObject->_tickCallback.Invoke(burtcObject->_timeDelta);
    }
}

void Burtc::Initialize()
{
    burtcInterruptSemaphore = System::CreateBinarySemaphore();

    if (OS_RESULT_FAILED(this->_task.Create()))
    {
        LOG(LOG_LEVEL_ERROR, "Error. Cannot create burtc task");
    }

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
