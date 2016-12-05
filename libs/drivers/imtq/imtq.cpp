/**
@file libs/drivers/imtq/imtq.cpp Driver for iMTQ module.

@remarks Based on ICD Issue 1.5 2015-07-22
*/
#include "imtq.h"
#include <stdnoreturn.h>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include "base/os.h"
#include "base/reader.h"
#include "base/writer.h"
#include "logger/logger.h"
#include "system.h"

using std::uint8_t;
using gsl::span;

namespace devices
{
    namespace imtq
    {
        constexpr int maximumCommandOutputLength = 9,
                      commandResponseLength = 2;


		Status::Status(std::uint8_t val) : value{val}
		{
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

        ImtqDriver::ImtqDriver(I2CBus& i2cbus) : i2cbus{i2cbus}
        {
        }

        bool ImtqDriver::SendNoOperation()
        {
            return SendCommand(OpCode::NoOperation);
        }

        bool ImtqDriver::SendCommand(OpCode opcode)
        {
        	uint8_t op = static_cast<uint8_t>(opcode);
            return SendCommand(span<uint8_t>(&op, 1));
        }

        bool ImtqDriver::SendCommand(OpCode opcode, span<const uint8_t> params)
        {
            UNREFERENCED_PARAMETER(opcode);
            UNREFERENCED_PARAMETER(params);
            return true;
        }

        bool ImtqDriver::SendCommand(span<const uint8_t> params)
        {
            std::array<uint8_t, commandResponseLength> response;

            auto result = i2cbus.WriteRead(&i2cbus, I2Cadress,
                                           params.data(), params.size(),
                                           response.data(), response.size());

            Status status{response[1]};

            if (result != I2CResultOK ||
            	status.CmdError() != Status::Error::Accepted ||
                response[0] != params[0])
            {
                return false;
            }
            return true;
        }
    }
}
