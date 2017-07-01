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
            error_counter::ErrorCounting& errorCounting, drivers::i2c::II2CBus& controllerABus, drivers::i2c::II2CBus& controllerBBus)
            : _error(errorCounting), _controllerABus(controllerABus), _controllerBBus(controllerBBus)
        {
        }

        Option<hk::ControllerATelemetry> EPSDriver::ReadHousekeepingA()
        {
            std::array<std::uint8_t, 1> command{0x0};
            std::array<std::uint8_t, 72> response;

            auto result = this->WriteRead(Controller::A, command, response);

            if (result != I2CResult::OK)
            {
                this->_error.Failure();
                return None<hk::ControllerATelemetry>();
            }

            hk::ControllerATelemetry housekeeping;
            Reader r(response);

            if (!housekeeping.ReadFrom(r))
            {
                this->_error.Failure();
                return None<hk::ControllerATelemetry>();
            }

            if (housekeeping.WhoAmI != ControllerAId)
            {
                this->_error.Failure();
                return None<hk::ControllerATelemetry>();
            }

            this->_error.Success();
            return Some(housekeeping);
        }

        Option<hk::ControllerBTelemetry> EPSDriver::ReadHousekeepingB()
        {
            std::array<std::uint8_t, 1> command{0x0};
            std::array<std::uint8_t, 16> response;

            auto result = this->WriteRead(Controller::B, command, response);

            if (result != I2CResult::OK)
            {
                this->_error.Failure();
                return None<hk::ControllerBTelemetry>();
            }

            hk::ControllerBTelemetry housekeeping;
            Reader r(response);

            if (!housekeeping.ReadFrom(r))
            {
                this->_error.Failure();
                return None<hk::ControllerBTelemetry>();
            }

            if (housekeeping.WhoAmI != ControllerBId)
            {
                this->_error.Failure();
                return None<hk::ControllerBTelemetry>();
            }

            this->_error.Success();
            return Some(housekeeping);
        }

        bool EPSDriver::PowerCycle(Controller controller)
        {
            LOGF(LOG_LEVEL_WARNING, "Triggering power cycle (%s)", controller == Controller::A ? "A" : "B");
            std::array<std::uint8_t, 1> command{num(Command::PowerCycle)};

            if (this->Write(controller, command) != I2CResult::OK)
            {
                this->_error.Failure();
                return false;
            }

            System::SleepTask(PowerCycleTimeout);

            this->_error.Failure();

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
                this->_error.Failure();
                return ErrorCode::CommunicationFailure;
            }

            return GetErrorCode(controller) >> this->_error;
        }

        ErrorCode EPSDriver::DisableLCL(LCL lcl)
        {
            auto controller = (num(lcl) & 0xF0) == 0 ? Controller::A : Controller::B;
            auto lclId = static_cast<std::uint8_t>((num(lcl) & 0x0F));

            std::array<std::uint8_t, 2> command{num(Command::DisableLCL), lclId};

            if (this->Write(controller, command) != I2CResult::OK)
            {
                this->_error.Failure();
                return ErrorCode::CommunicationFailure;
            }

            return GetErrorCode(controller) >> this->_error;
        }

        bool EPSDriver::DisableOverheatSubmode(Controller controller)
        {
            std::array<std::uint8_t, 1> command{num(Command::DisableOverheatSubmode)};
            return (this->Write(controller, command) == I2CResult::OK) >> this->_error;
        }

        ErrorCode EPSDriver::EnableBurnSwitch(bool main, BurnSwitch burnSwitch)
        {
            std::array<std::uint8_t, 2> command{num(Command::EnableBurnSwitch), num(burnSwitch)};

            auto controller = main ? Controller::A : Controller::B;

            if (this->Write(controller, command) != I2CResult::OK)
            {
                this->_error.Failure();
                return ErrorCode::CommunicationFailure;
            }

            return GetErrorCode(controller) >> this->_error;
        }

        ErrorCode EPSDriver::GetErrorCode(Controller controller)
        {
            std::array<std::uint8_t, 1> command;

            if (controller == Controller::A)
            {
                command[0] = 0x4B;
            }
            else
            {
                command[0] = 0x07;
            }

            std::array<std::uint8_t, 1> response;

            auto r = this->WriteRead(controller, command, response) >> this->_error;

            if (r != I2CResult::OK)
            {
                return ErrorCode::CommunicationFailure;
            }

            return static_cast<ErrorCode>(response[0]);
        }

        ErrorCode EPSDriver::ResetWatchdog(Controller controller)
        {
            std::array<std::uint8_t, 1> command{num(Command::ResetWatchdog)};

            if (this->Write(controller, command) != I2CResult::OK)
            {
                this->_error.Failure();
                return ErrorCode::CommunicationFailure;
            }

            return GetErrorCode(controller) >> this->_error;
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
