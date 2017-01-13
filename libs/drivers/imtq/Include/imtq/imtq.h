#ifndef SRC_DEVICES_IMTQ_H_
#define SRC_DEVICES_IMTQ_H_

#include <stdbool.h>
#include <array>
#include <cstdint>
#include <chrono>
#include <gsl/span>

#include "base/os.h"
#include "i2c/i2c.h"

using std::uint8_t;

namespace devices
{
    namespace imtq
    {
        constexpr std::uint8_t I2Cadress = 0x10;

        enum class OpCode : uint8_t
        {
            // ----- Commands -----
            SoftwareReset = 0xAA,
            NoOperation = 0x02,
            CancelOperation = 0x03,
            StartMTMMeasurement = 0x04,
            StartActuationCurrent = 0x05,
            StartActuationDipole = 0x06,
            StartActuationPWM = 0x07,
            StartSelfTest = 0x08,
            StartBDOT = 0x09,

            // ----- Data requests -----
            GetIMTQSystemState = 0x41,
            GetRawMTMData = 0x42,
            GetCalibratedMTMData = 0x43,
            GetCoilCurrent = 0x44,
            GetCoilTemperatures = 0x45,
            GetCommandedActuationDipole = 0x46,
            GetSelfTest = 0x47,
            GetDetumbleData = 0x48,
            GetRAWHousekeepingData = 0x49,
            GetEngineeringHousekeepingData = 0x4A,

            // ----- Configuration -----
            GetParameter = 0x81,
            SetParameter = 0x82,
            ResetParameter = 0x83
        };

        class Status
        {
          public:
        	enum class Error
			{
        		Accepted = 0,
        		Rejected = 1,
				InvalidCommandCode = 2,
				ParameterMissing = 3,
				ParameterInvalid = 4,
				CommandUnavailableInCurrentMode = 5,
				InternalError = 7
        	};

        	Status(std::uint8_t);
        	bool IsNew();
        	bool InvalidX();
        	bool InvalidY();
        	bool InvalidZ();
        	Error CmdError();

          private:
        	std::uint8_t value;
        };

        class Current
        {
          public:
        	std::uint16_t getIn0dot1miliAmpsStep();
        	void setIn0dot1miliAmpsStep(std::uint16_t value);

        	std::uint16_t getInMiliAmpere();
        	void setInMiliAmpere(std::uint16_t value);

          private:
        	// representation: 1 LSB = 1e-4 A
        	std::uint16_t value;
        };

        class ImtqDriver final
        {
          public:
            ImtqDriver(drivers::i2c::II2CBus& i2cbus);

            bool SendNoOperation();
            bool SoftwareReset();
            bool CancelOperation();
            bool StartMTMMeasurement();
            bool StartActuationCurrent(std::array<Current, 3> current, std::chrono::milliseconds duration);

          private:
            drivers::i2c::II2CBus& i2cbus;

            bool SendCommand(OpCode opcode);
            bool SendCommand(OpCode opcode, gsl::span<const std::uint8_t> params);
            bool SendCommand(gsl::span<const std::uint8_t> params);
        };
    }
}

#endif /* SRC_DEVICES_IMTQ_H_ */
