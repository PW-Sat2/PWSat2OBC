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
        template <error_counter::Device Device>
        static constexpr I2CResult operator>>(I2CResult result, error_counter::ErrorCounter<Device>& counter)
        {
            (result == I2CResult::OK) >> counter;

            return result;
        }

        template <error_counter::Device Device>
        static constexpr ErrorCode operator>>(ErrorCode result, error_counter::ErrorCounter<Device>& counter)
        {
            (result == ErrorCode::NoError) >> counter;

            return result;
        }

        static constexpr auto PowerCycleTimeout = 3s + 1s;

        enum class Command
        {
            PowerCycle = 0xE0,
            EnableLCL = 0xE1,
            DisableLCL = 0xE2,
            EnableBurnSwitch = 0xE3,
            DisableOverheatSubmode = 0xE4,
            ResetWatchdog = 0xE5
        };

        EPSDriver::EPSDriver(
            drivers::i2c::II2CBus& controllerABus, drivers::i2c::II2CBus& controllerBBus)
            : _controllerABus(controllerABus), _controllerBBus(controllerBBus)
        {
        }

        Option<hk::ControllerATelemetry> EPSDriver::ReadHousekeepingA()
        {
            return None<hk::ControllerATelemetry>();
        }

        Option<hk::ControllerBTelemetry> EPSDriver::ReadHousekeepingB()
        {
            return None<hk::ControllerBTelemetry>();
        }

        bool EPSDriver::PowerCycle(Controller controller)
        {
            LOGF(LOG_LEVEL_WARNING, "Triggering power cycle (%s)", controller == Controller::A ? "A" : "B");
            std::array<std::uint8_t, 1> command{num(Command::PowerCycle)};

            if (this->Write(controller, command) != I2CResult::OK)
            {
                return false;
            }

            System::SleepTask(PowerCycleTimeout);

            LOGF(LOG_LEVEL_ERROR, "Power cycle failed (%s)", controller == Controller::A ? "A" : "B");

            return false;
        }

        bool EPSDriver::PowerCycle()
        {
            this->PowerCycle(Controller::A);
            this->PowerCycle(Controller::B);
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

            return ErrorCode::NoError;
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

            return ErrorCode::NoError;
        }

        bool EPSDriver::DisableOverheatSubmode(Controller controller)
        {
            std::array<std::uint8_t, 1> command{num(Command::DisableOverheatSubmode)};
            return (this->Write(controller, command) == I2CResult::OK);
        }

        ErrorCode EPSDriver::EnableBurnSwitch(bool main, BurnSwitch burnSwitch)
        {
            std::array<std::uint8_t, 2> command{num(Command::EnableBurnSwitch), num(burnSwitch)};

            auto controller = main ? Controller::A : Controller::B;

            if (this->Write(controller, command) != I2CResult::OK)
            {
                return ErrorCode::CommunicationFailure;
            }

            return ErrorCode::NoError;
        }

        ErrorCode EPSDriver::ResetWatchdog(Controller controller)
        {
            std::array<std::uint8_t, 1> command{num(Command::ResetWatchdog)};

            if (this->Write(controller, command) != I2CResult::OK)
            {
                return ErrorCode::CommunicationFailure;
            }

            return ErrorCode::NoError;
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
