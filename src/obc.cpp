#include "obc.h"
#include "io_map.h"

OBC::OBC()
    : //
      I2CBuses{
          {I2C0, I2C0_BUS_LOCATION, I2C0_BUS_PORT, I2C0_BUS_SDA_PIN, I2C0_BUS_SCL_PIN, cmuClock_I2C0, I2C0_IRQn},
          {I2C1, I2C1_BUS_LOCATION, I2C1_BUS_PORT, I2C1_BUS_SDA_PIN, I2C1_BUS_SCL_PIN, cmuClock_I2C1, I2C1_IRQn} //
      },
      I2C(I2CBuses[I2C_SYSTEM_BUS].ErrorHandling, I2CBuses[I2C_PAYLOAD_BUS].ErrorHandling), //
      I2CFallback(I2C),                                                                     //
      Communication(I2C.Bus),                                                               //
      terminal(this->IO)
{
}

void OBC::Initialize()
{
    this->I2CBuses[0].Driver.Initialize();
    this->I2CBuses[1].Driver.Initialize();

    this->Communication.Initialize();
}

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
