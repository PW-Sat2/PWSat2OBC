#include "hardware.h"

#include "obc.h"

using drivers::i2c::II2CBus;
using drivers::i2c::I2CResult;
using drivers::i2c::I2CAddress;

I2CSingleBus::I2CSingleBus(I2C_TypeDef* hw,
    uint16_t location,
    GPIO_Port_TypeDef port,
    uint16_t sdaPin,
    uint16_t sclPin,
    CMU_Clock_TypeDef clock,
    IRQn_Type irq)
    : //
      Driver(hw, location, port, sdaPin, sclPin, clock, irq),
      ErrorHandling(Driver, I2CErrorHandler, &Main.PowerControlInterface)
{
}

I2CResult I2CSingleBus::I2CErrorHandler(II2CBus& bus, I2CResult result, I2CAddress address, void* context)
{
    UNREFERENCED_PARAMETER(bus);
    UNREFERENCED_PARAMETER(address);

    auto power = reinterpret_cast<PowerControl*>(context);

    if (result == I2CResult::ClockLatched)
    {
        LOG(LOG_LEVEL_FATAL, "SCL latched. Triggering power cycle");
        power->TriggerSystemPowerCycle(power);
        return result;
    }

    return result;
}

OBCHardwareI2C::OBCHardwareI2C()
    : //
      Peripherals{
          {I2C0, I2C0_BUS_LOCATION, I2C0_BUS_PORT, I2C0_BUS_SDA_PIN, I2C0_BUS_SCL_PIN, cmuClock_I2C0, I2C0_IRQn},
          {I2C1, I2C1_BUS_LOCATION, I2C1_BUS_PORT, I2C1_BUS_SDA_PIN, I2C1_BUS_SCL_PIN, cmuClock_I2C1, I2C1_IRQn} //
      },
      Buses(Peripherals[I2C_SYSTEM_BUS].ErrorHandling, Peripherals[I2C_PAYLOAD_BUS].ErrorHandling), //
      Fallback(Buses)                                                                               //
{
}

void OBCHardwareI2C::Initialize()
{
    this->Peripherals[0].Driver.Initialize();
    this->Peripherals[1].Driver.Initialize();
}

void OBCHardware::Initialize()
{
    this->I2C.Initialize();
    this->SPI.Initialize();
}
