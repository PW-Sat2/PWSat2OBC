/**
@file libs/drivers/imtq/imtq.cpp Driver for iMTQ module.

@remarks Based on ICD Issue 1.5 2015-07-22
*/
#include "imtq.h"
#include <stdnoreturn.h>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include "base/os.h"
#include "base/reader.h"
#include "base/writer.h"
#include "logger/logger.h"
#include "system.h"

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using gsl::span;
using drivers::i2c::I2CResult;

namespace devices
{
    namespace imtq
    {
        constexpr int maximumCommandOutputLength = 9,
                      commandResponseLength = 2;

        // ------------------------- status -------------------------

		Status::Status(std::uint8_t val) : value{val}
		{
		}

		bool Status::IsNew()
		{
			return value & 0b10000000;
		}

		bool Status::InvalidX()
		{
			return value & 0b01000000;
		}
		bool Status::InvalidY()
		{
			return value & 0b00100000;
		}
		bool Status::InvalidZ()
		{
			return value & 0b00010000;
		}
		Status::Error Status::CmdError()
		{
			return static_cast<Status::Error>(value & 0b00001111);
		}

		// ------------------------- Current -------------------------

		uint16_t CurrentMeasurement::getIn0dot1miliAmpsStep()
		{
			return this->value;
		}
		void CurrentMeasurement::setIn0dot1miliAmpsStep(uint16_t value)
		{
			this->value = value;
		}

		uint16_t CurrentMeasurement::getInMiliAmpere()
		{
			return this->value/10;
		}
		void CurrentMeasurement::setInMiliAmpere(uint16_t value)
		{
			this->value = value*10;
		}

		// ------------------------- Public functions -------------------------

        ImtqDriver::ImtqDriver(drivers::i2c::II2CBus& i2cbus) : i2cbus{i2cbus}
        {
        }

        bool ImtqDriver::SendNoOperation()
        {
            return SendCommand(OpCode::NoOperation);
        }

        bool ImtqDriver::SoftwareReset()
        {
        	uint8_t opcode = static_cast<uint8_t>(OpCode::SoftwareReset);

            std::array<uint8_t, commandResponseLength> response;

            auto result = i2cbus.WriteRead(I2Cadress,
                                           gsl::span<const uint8_t, 1>(&opcode, 1),
                                           response);

            if (result == I2CResult::Nack)
            {
            	return true;
            }
            if (result != I2CResult::OK)
			{
				return false;
			}
            if (response[0] == 0xFF && response[1] == 0xFF)
            {
            	return true;
            }
            return false;
        }

        bool ImtqDriver::CancelOperation()
        {
        	return SendCommand(OpCode::CancelOperation);
        }

        bool ImtqDriver::StartMTMMeasurement()
		{
			return SendCommand(OpCode::StartMTMMeasurement);
		}

        bool ImtqDriver::StartActuationCurrent(Vector3<CurrentMeasurement> current, std::chrono::milliseconds duration)
        {
        	std::array<uint8_t, 8> parameters;
        	Writer writer;
        	WriterInitialize(&writer, parameters.data(), parameters.size());
        	WriterWriteWordLE(&writer, current[0].getIn0dot1miliAmpsStep());
        	WriterWriteWordLE(&writer, current[1].getIn0dot1miliAmpsStep());
        	WriterWriteWordLE(&writer, current[2].getIn0dot1miliAmpsStep());
        	WriterWriteWordLE(&writer, duration.count());

        	return this->SendCommand(OpCode::StartActuationCurrent, parameters);
        }

        bool ImtqDriver::StartActuationDipole(Vector3<Dipole> dipole, std::chrono::milliseconds duration)
        {
        	std::array<uint8_t, 8> parameters;
			Writer writer;
			WriterInitialize(&writer, parameters.data(), parameters.size());
			WriterWriteWordLE(&writer, dipole[0].getIn0dot1miliAmpsPerMeterSq());
			WriterWriteWordLE(&writer, dipole[1].getIn0dot1miliAmpsPerMeterSq());
			WriterWriteWordLE(&writer, dipole[2].getIn0dot1miliAmpsPerMeterSq());
			WriterWriteWordLE(&writer, duration.count());

			return this->SendCommand(OpCode::StartActuationDipole, parameters);
        }

        bool ImtqDriver::StartAllAxisSelfTest()
        {
        	// 0x00 means test all axis test
        	std::array<uint8_t, 1> parameters = {0x00};
        	return this->SendCommand(OpCode::StartSelfTest, parameters);
        }

        bool ImtqDriver::StartBDotDetumbling(std::chrono::seconds duration)
        {
        	std::array<uint8_t, 2> parameters;
        	Writer writer;
        	WriterInitialize(&writer, parameters.data(), parameters.size());
        	WriterWriteWordLE(&writer, duration.count());

        	return this->SendCommand(OpCode::StartBDOT, parameters);
        }

        // ------------------------- Private -------------------------

        bool ImtqDriver::SendCommand(OpCode opcode)
        {
        	uint8_t op = static_cast<uint8_t>(opcode);
            return SendCommand(span<uint8_t>(&op, 1));
        }

        bool ImtqDriver::SendCommand(OpCode opcode, span<const uint8_t> params)
        {
            std::array<uint8_t, maximumCommandOutputLength> output;
            output[0] = static_cast<uint8_t>(opcode);
            std::copy(params.begin(), params.end(), output.begin()+1);

            return this->SendCommand(output);
        }

        bool ImtqDriver::SendCommand(span<const uint8_t> params)
        {
            std::array<uint8_t, commandResponseLength> response;

            auto result = i2cbus.WriteRead(I2Cadress, params, response);

            Status status{response[1]};

            if (result != I2CResult::OK ||
            	status.CmdError() != Status::Error::Accepted ||
                response[0] != params[0])
            {
                return false;
            }
            return true;
        }
    }
}
