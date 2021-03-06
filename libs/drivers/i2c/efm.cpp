#include <cassert>
#include <em_cmu.h>
#include <em_gpio.h>

#include "base/os.h"
#include "logger/logger.h"
#include "mcu/io_map.h"

#include "efm.h"

using namespace drivers::i2c;

bool I2CLowLevelBus::IsSclOrSdaLatched()
{
    return GPIO_PinInGet(this->_io.Port, this->_io.SCL) == 0 || GPIO_PinInGet(this->_io.Port, this->_io.SDA) == 0;
}

I2CResult I2CLowLevelBus::ExecuteTransfer(I2C_TransferSeq_TypeDef* seq)
{
    Lock lock(this->_lock, InfiniteTimeout);

    if (!lock())
    {
        LOGF(LOG_LEVEL_ERROR, "[I2C] Taking semaphore failed. Address: %X", seq->addr);
        return I2CResult::Failure;
    }

    if (this->IsSclOrSdaLatched())
    {
        LOG(LOG_LEVEL_FATAL, "[I2C] SCL or SDA already latched");
        return I2CResult::LineAlreadyLatched;
    }

    auto hw = reinterpret_cast<I2C_TypeDef*>(this->HWInterface);

    auto rawResult = I2C_TransferInit(hw, seq);

    if (rawResult != i2cTransferInProgress)
    {
        return (I2CResult)rawResult;
    }

    if (OS_RESULT_FAILED(this->_resultQueue.Pop(rawResult, std::chrono::seconds(static_cast<uint64_t>(io_map::I2C::Timeout)))))
    {
        I2CResult ret = I2CResult::Timeout;

        LOG(LOG_LEVEL_ERROR, "Didn't received i2c transfer result");

        hw->CMD = I2C_CMD_STOP | I2C_CMD_ABORT;

        while (has_flag(hw->STATUS, I2C_STATUS_PABORT))
        {
        }

        if (this->IsSclOrSdaLatched())
        {
            LOG(LOG_LEVEL_ERROR, "SCL or SDA latched at low level");

            ret = I2CResult::LineLatched;
        }

        return ret;
    }

    return (I2CResult)rawResult;
}

I2CResult I2CLowLevelBus::Write(const I2CAddress address, gsl::span<const uint8_t> inData)
{
    assert((address & 0b10000000) == 0);

    I2C_TransferSeq_TypeDef seq;
    seq.addr = (address << 1);
    seq.flags = I2C_FLAG_WRITE;
    seq.buf[0].len = inData.length();
    seq.buf[0].data = const_cast<uint8_t*>(inData.data());
    seq.buf[1].len = 0;
    seq.buf[1].data = nullptr;

    return ExecuteTransfer(&seq);
}

I2CResult I2CLowLevelBus::Read(const I2CAddress address, gsl::span<uint8_t> outData)
{
    assert((address & 0b10000000) == 0);

    I2C_TransferSeq_TypeDef seq;
    seq.addr = (address << 1);
    seq.flags = I2C_FLAG_READ;
    seq.buf[0].len = outData.length();
    seq.buf[0].data = const_cast<uint8_t*>(outData.data());
    seq.buf[1].len = 0;
    seq.buf[1].data = nullptr;

    return ExecuteTransfer(&seq);
}

I2CResult I2CLowLevelBus::WriteRead(const I2CAddress address, gsl::span<const uint8_t> inData, gsl::span<uint8_t> outData)
{
    assert((address & 0b10000000) == 0);

    I2C_TransferSeq_TypeDef seq;
    seq.addr = (address << 1);
    seq.flags = I2C_FLAG_WRITE_READ;
    seq.buf[0].len = inData.length();
    seq.buf[0].data = const_cast<uint8_t*>(inData.data());
    seq.buf[1].len = outData.length();
    seq.buf[1].data = outData.data();

    return ExecuteTransfer(&seq);
}

I2CLowLevelBus::I2CLowLevelBus(I2C_TypeDef* hw, //
    uint16_t location,
    GPIO_Port_TypeDef port,
    uint16_t sdaPin,
    uint16_t sclPin,
    CMU_Clock_TypeDef clock,
    IRQn_Type irq)
    : HWInterface(hw), _io{clock, irq, location, port, sclPin, sdaPin}
{
}

void I2CLowLevelBus::Initialize()
{
    this->_resultQueue.Create();

    this->_lock = System::CreateBinarySemaphore();
    System::GiveSemaphore(this->_lock);

    CMU_ClockEnable(this->_io.Clock, true);

    GPIO_PinModeSet(this->_io.Port, this->_io.SDA, gpioModeWiredAndPullUpFilter, 1);
    GPIO_PinModeSet(this->_io.Port, this->_io.SCL, gpioModeWiredAndPullUpFilter, 1);

    I2C_Init_TypeDef init = I2C_INIT_DEFAULT;
    init.clhr = i2cClockHLRStandard;
    init.enable = true;

    auto hw = reinterpret_cast<I2C_TypeDef*>(this->HWInterface);

    I2C_Init(hw, &init);
    hw->ROUTE = I2C_ROUTE_SCLPEN | I2C_ROUTE_SDAPEN | this->_io.Location;

    I2C_IntEnable(hw, I2C_IEN_TXC);

    NVIC_SetPriority(this->_io.IRQn, io_map::I2C::InterruptPriority);
    NVIC_EnableIRQ(this->_io.IRQn);
}

void I2CLowLevelBus::IRQHandler()
{
    auto status = I2C_Transfer(reinterpret_cast<I2C_TypeDef*>(this->HWInterface));

    if (status == i2cTransferInProgress)
    {
        System::EndSwitchingISR();
        return;
    }

    if (OS_RESULT_FAILED(this->_resultQueue.PushISR(status)))
    {
        LOG_ISR(LOG_LEVEL_ERROR, "Error queueing i2c result");
    }

    System::EndSwitchingISR();
}
