#include "eps.h"
#include "base/os.h"
#include "base/reader.h"
#include "gsl/span"
#include "i2c/i2c.h"
#include "logger/logger.h"
#include "system.h"

using drivers::i2c::II2CBus;
using drivers::i2c::I2CResult;
using namespace std::chrono_literals;

namespace devices
{
    namespace eps
    {
        static constexpr auto PowerCycleTimeout = 3s + 1s;

        enum class Command
        {
            PowerCycle = 0xE0,
            EnableLCL = 0xE1,
            DisableLCL = 0xE2,
            EnableBurnSwitch = 0xE3,
            DisableOverheatSubmode = 0xE4,
        };

        EPSDriver::EPSDriver(drivers::i2c::II2CBus& controllerABus, drivers::i2c::II2CBus& controllerBBus)
            : _controllerABus(controllerABus), _controllerBBus(controllerBBus)
        {
        }

        Option<hk::HouseheepingControllerA> EPSDriver::ReadHousekeepingA()
        {
            std::array<std::uint8_t, 1> command{0x0};
            std::array<std::uint8_t, 72> response;

            auto result = this->WriteRead(Controller::A, command, response);

            if (result != I2CResult::OK)
            {
                return None<hk::HouseheepingControllerA>();
            }

            hk::HouseheepingControllerA housekeeping;
            Reader r(response);
            r.ReadByte(); // error flag register

            if (!housekeeping.ReadFrom(r))
            {
                return None<hk::HouseheepingControllerA>();
            }

            return Some(housekeeping);
        }

        Option<hk::HouseheepingControllerB> EPSDriver::ReadHousekeepingB()
        {
            std::array<std::uint8_t, 1> command{0x0};
            std::array<std::uint8_t, 16> response;

            auto result = this->WriteRead(Controller::B, command, response);

            if (result != I2CResult::OK)
            {
                return None<hk::HouseheepingControllerB>();
            }

            hk::HouseheepingControllerB housekeeping;
            Reader r(response);

            r.ReadByte(); // error flag register

            if (!housekeeping.ReadFrom(r))
            {
                return None<hk::HouseheepingControllerB>();
            }

            return Some(housekeeping);
        }

        bool EPSDriver::PowerCycleA()
        {
            LOG(LOG_LEVEL_WARNING, "Triggering power cycle (A)");
            std::array<std::uint8_t, 1> command{num(Command::PowerCycle)};

            if (this->Write(Controller::A, command) != I2CResult::OK)
            {
                return false;
            }

            System::SleepTask(PowerCycleTimeout);

            LOG(LOG_LEVEL_ERROR, "Power cycle failed (A)");

            return false;
        }

        bool EPSDriver::PowerCycleB()
        {
            LOG(LOG_LEVEL_WARNING, "Triggering power cycle (B)");
            std::array<std::uint8_t, 1> command{num(Command::PowerCycle)};

            if (this->Write(Controller::B, command) != I2CResult::OK)
            {
                return false;
            }

            System::SleepTask(PowerCycleTimeout);

            LOG(LOG_LEVEL_ERROR, "Power cycle failed (B)");

            return false;
        }

        bool EPSDriver::PowerCycle()
        {
            this->PowerCycleA();
            this->PowerCycleB();
            return false;
        }

        ErrorCode EPSDriver::EnableLCL(LCL lcl)
        {
            auto controller = (num(lcl) & 0xF0) == 0 ? Controller::A : Controller::B;
            auto lclId = static_cast<std::uint8_t>((num(lcl) & 0x0F));

            std::array<std::uint8_t, 2> command{num(Command::EnableLCL), lclId};

            if (this->Write(controller, command) != I2CResult::OK)
            {
                return ErrorCode::CommunicationFailure;
            }

            return GetErrorCode(controller);
        }

        ErrorCode EPSDriver::DisableLCL(LCL lcl)
        {
            auto controller = (num(lcl) & 0xF0) == 0 ? Controller::A : Controller::B;
            auto lclId = static_cast<std::uint8_t>((num(lcl) & 0x0F));

            std::array<std::uint8_t, 2> command{num(Command::DisableLCL), lclId};

            if (this->Write(controller, command) != I2CResult::OK)
            {
                return ErrorCode::CommunicationFailure;
            }

            return GetErrorCode(controller);
        }

        bool EPSDriver::DisableOverheatSubmodeA()
        {
            std::array<std::uint8_t, 1> command{num(Command::DisableOverheatSubmode)};
            return this->Write(Controller::A, command) == I2CResult::OK;
        }

        bool EPSDriver::DisableOverheatSubmodeB()
        {
            std::array<std::uint8_t, 1> command{num(Command::DisableOverheatSubmode)};
            return this->Write(Controller::B, command) == I2CResult::OK;
        }

        ErrorCode EPSDriver::EnableBurnSwitch(bool main, BurnSwitch burnSwitch)
        {
            std::array<std::uint8_t, 2> command{num(Command::EnableBurnSwitch), num(burnSwitch)};

            auto controller = main ? Controller::A : Controller::B;

            if (this->Write(controller, command) != I2CResult::OK)
            {
                return ErrorCode::CommunicationFailure;
            }

            return GetErrorCode(controller);
        }

        ErrorCode EPSDriver::GetErrorCode(Controller controller)
        {
            std::array<std::uint8_t, 1> command{0x0};
            std::array<std::uint8_t, 1> response;

            auto r = this->WriteRead(controller, command, response);

            if (r != I2CResult::OK)
            {
                return ErrorCode::CommunicationFailure;
            }

            return static_cast<ErrorCode>(response[0]);
        }

        ErrorCode EPSDriver::GetErrorCodeA()
        {
            return GetErrorCode(Controller::A);
        }

        ErrorCode EPSDriver::GetErrorCodeB()
        {
            return GetErrorCode(Controller::B);
        }

        I2CResult EPSDriver::Write(Controller controller, const gsl::span<std::uint8_t> inData)
        {
            switch (controller)
            {
                case Controller::A:
                    return this->_controllerABus.Write(ControllerA, inData);
                case Controller::B:
                    return this->_controllerBBus.Write(ControllerB, inData);
                default:
                    return I2CResult::Failure;
            }
        }

        I2CResult EPSDriver::WriteRead(Controller controller, gsl::span<const uint8_t> inData, gsl::span<uint8_t> outData)
        {
            switch (controller)
            {
                case Controller::A:
                    return this->_controllerABus.WriteRead(ControllerA, inData, outData);
                case Controller::B:
                    return this->_controllerBBus.WriteRead(ControllerB, inData, outData);
                default:
                    return I2CResult::Failure;
            }
        }
    }
}
