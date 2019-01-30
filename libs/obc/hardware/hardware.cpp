#include "hardware.h"

using drivers::i2c::II2CBus;
using drivers::i2c::I2CResult;
using drivers::i2c::I2CAddress;
using io_map::I2C;
using io_map::I2C_0;
using io_map::I2C_1;

using namespace obc;

I2CSingleBus::I2CSingleBus(I2C_TypeDef* hw,
    uint16_t location,
    GPIO_Port_TypeDef port,
    uint16_t sdaPin,
    uint16_t sclPin,
    CMU_Clock_TypeDef clock,
    IRQn_Type irq,
    services::power::IPowerControl& powerControl)
    : //
      Driver(hw, location, port, sdaPin, sclPin, clock, irq),
      ErrorHandling(Driver, I2CErrorHandler, &powerControl)
{
}

I2CResult I2CSingleBus::I2CErrorHandler(II2CBus& bus, I2CResult result, I2CAddress address, void* context)
{
    UNREFERENCED_PARAMETER(bus);
    UNREFERENCED_PARAMETER(address);

    auto power = reinterpret_cast<services::power::IPowerControl*>(context);

    if (result == I2CResult::LineLatched)
    {
        LOG(LOG_LEVEL_FATAL, "SCL/SDA latched. Triggering power cycle");
        power->PowerCycle();
        return result;
    }

    return result;
}

OBCHardwareI2C::OBCHardwareI2C(services::power::IPowerControl& powerControl)
    : //
      Peripherals{
          {I2C0, I2C_0::Location, I2C_0::SDA::Port, I2C_0::SDA::PinNumber, I2C_0::SCL::PinNumber, cmuClock_I2C0, I2C0_IRQn, powerControl},
          {I2C1, I2C_1::Location, I2C_1::SDA::Port, I2C_1::SDA::PinNumber, I2C_1::SCL::PinNumber, cmuClock_I2C1, I2C1_IRQn, powerControl} //
      },
      Buses(Peripherals[I2C::SystemBus].ErrorHandling, Peripherals[I2C::PayloadBus].ErrorHandling), //
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
    this->Pins.Initialize();
    this->I2C.Initialize();

    ResetEps();

    this->Terminal.Initialize();
    this->Terminal.LineEditing(true);

    this->Camera.Initialize();

    this->SPI.Initialize();

    this->FlashDriver.Initialize();
    this->PayloadDriver.Initialize();

    this->imtqTelemetryCollector.Initialize();

    this->SunS.Initialize();

    this->Burtc.Initialize();

    this->CommDriver.Initialize();

    this->rtc.Initialize();

    this->Gyro.init();
}

void OBCHardware::ResetEps()
{
    this->EPS.DisableLCL(devices::eps::LCL::SunS);
    this->EPS.DisableLCL(devices::eps::LCL::CamNadir);
    this->EPS.DisableLCL(devices::eps::LCL::CamWing);
    this->EPS.DisableLCL(devices::eps::LCL::SENS);
    this->EPS.DisableLCL(devices::eps::LCL::AntennaMain);
    this->EPS.DisableLCL(devices::eps::LCL::AntennaRed);
}

OBCHardware::OBCHardware(
    error_counter::ErrorCounting& errorCounting, services::power::IPowerControl& powerControl, TimeAction& burtcTickHandler)
    : I2C(powerControl),                                                                  //
      FlashDriver(io_map::ProgramFlash::FlashBase),                                       //
      Burtc(burtcTickHandler),                                                            //
      FramSpi{                                                                            //
          {SPI, Pins.Fram1ChipSelect},                                                    //
          {SPI, Pins.Fram2ChipSelect},                                                    //
          {SPI, Pins.Fram3ChipSelect}},                                                   //
      PersistentStorage{errorCounting,                                                    //
          {&FramSpi[0],                                                                   //
              &FramSpi[1],                                                                //
              &FramSpi[2]}},                                                              //
      Gyro(I2C.Buses.Payload),                                                            //
      EPS(errorCounting, this->I2C.Buses.Bus, this->I2C.Buses.Payload),                   //
    //   antennaDriver(errorCounting, &antennaMiniport, &I2C.Buses.Bus, &I2C.Buses.Payload), //
      Imtq(errorCounting, I2C.Buses.Bus),                                                 //
      imtqTelemetryCollector(Imtq),                                                       //
      SunSInterruptDriver(this->Pins.SunSInterrupt),                                      //
      SunS(errorCounting, I2C.Buses.Payload, SunSInterruptDriver),                        //
      rtc(errorCounting, I2C.Buses.Payload),                                              //
      CommDriver(errorCounting, I2C.Buses.Bus),                                           //
      PayloadInterruptDriver(this->Pins.PayloadInterrupt),                                //
      PayloadDriver(errorCounting, this->I2C.Buses.Payload, PayloadInterruptDriver),      //
      PayloadDeviceDriver(PayloadDriver)
{
}
