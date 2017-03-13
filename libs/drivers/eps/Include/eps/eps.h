#ifndef SRC_DEVICES_EPS_H_
#define SRC_DEVICES_EPS_H_

#include <stdbool.h>
#include <cstdint>
#include "base/reader.h"
#include "hk.hpp"
#include "i2c/forward.h"

void EpsInit(drivers::i2c::II2CBus* bus);
bool EpsOpenSail(void);

namespace devices
{
    namespace eps
    {
        enum class LCL : std::uint8_t
        {
            TKMain = 0x01,
            SunS = 0x02,
            CamNadir = 0x03,
            CamWing = 0x04,
            SENS = 0x05,
            AntennaMain = 0x06,
            TKRed = 0x11,
            AntennaRed = 0x12
        };

        enum class BurnSwitch : std::uint8_t
        {
            Sail = 0x1,
            SADS = 0x2
        };

        enum class ErrorCode : std::uint16_t
        {
            NoError = 0,
            OnFire = 0xAF,
            CommunicationFailure = 0xFF + 1
        };

        class EPSDriver
        {
          public:
            static constexpr drivers::i2c::I2CAddress ControllerA = 0b0110101;
            static constexpr drivers::i2c::I2CAddress ControllerB = 0b0110110;

            EPSDriver(drivers::i2c::I2CInterface& i2c);

            Option<hk::HouseheepingControllerA> ReadHousekeepingA();
            Option<hk::HouseheepingControllerB> ReadHousekeepingB();

            bool PowerCycleA();
            bool PowerCycleB();
            bool PowerCycle();

            ErrorCode EnableLCL(LCL lcl);
            ErrorCode DisableLCL(LCL lcl);

            bool DisableOverheatSubmodeA();
            bool DisableOverheatSubmodeB();

            ErrorCode EnableBurnSwitch(bool main, BurnSwitch burnSwitch);

            ErrorCode GetErrorCodeA();
            ErrorCode GetErrorCodeB();

          private:
            drivers::i2c::I2CInterface& _i2c;

            enum class Controller
            {
                A,
                B
            };

            ErrorCode GetErrorCode(Controller controller);
            drivers::i2c::I2CResult Write(Controller controller, const gsl::span<std::uint8_t> inData);
            drivers::i2c::I2CResult WriteRead(Controller controller, gsl::span<const uint8_t> inData, gsl::span<uint8_t> outData);
        };
    }
}

#endif /* SRC_DEVICES_EPS_H_ */
