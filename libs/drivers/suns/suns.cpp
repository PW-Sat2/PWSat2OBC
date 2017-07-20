#include "suns.hpp"
#include <stdnoreturn.h>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <cstring>
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
using devices::suns::OperationStatus;
using error_counter::AggregatedErrorCounter;

using namespace std::chrono_literals;

namespace devices
{
    namespace suns
    {
        SunSDriver::SunSDriver(error_counter::ErrorCounting& errors, drivers::i2c::II2CBus& i2cbus)
            : errors(errors), //
              i2cbus(i2cbus)  //
        {
        }

        OperationStatus SunSDriver::MeasureSunS(MeasurementData& data, uint8_t gain, uint8_t itime)
        {
            auto status = StartMeasurement(gain, itime);
            if (status != OperationStatus::OK)
            {
                return status;
            }

            System::SleepTask(4s);

            return GetMeasuredData(data);
        }

        OperationStatus SunSDriver::StartMeasurement(uint8_t gain, uint8_t itime)
        {
            ErrorReporter errorContext(this->errors);

            std::array<uint8_t, 3> output;
            Writer outputWriter{output};
            outputWriter.WriteByte(0x80);
            outputWriter.WriteByte(gain);
            outputWriter.WriteByte(itime);

            span<uint8_t> request = outputWriter.Capture();
            auto i2cstatusWrite = this->i2cbus.Write(I2Cadress, request);
            if (i2cstatusWrite != I2CResult::OK)
            {
                false >> errorContext.Counter();

                return OperationStatus::I2CWriteFailed;
            }

            return OperationStatus::OK;
        }

        OperationStatus SunSDriver::GetMeasuredData(MeasurementData& data)
        {
            ErrorReporter errorContext(this->errors);

            std::array<uint8_t, 536> response;

            auto i2cstatusRead = this->i2cbus.Read(I2Cadress, response);
            if (i2cstatusRead != I2CResult::OK)
            {
                false >> errorContext.Counter();

                return OperationStatus::I2CReadFailed;
            }

            Reader reader{response};

            auto opCode = reader.ReadByte();
            if (opCode != 0x80)
            {
                return OperationStatus::WrongOpcodeInResponse;
            }

            auto whoAmI = reader.ReadByte();
            if (whoAmI != 0x11)
            {
                return OperationStatus::WhoAmIMismatch;
            }

            data.status.ack = reader.ReadWordLE();
            data.status.presence = reader.ReadWordLE();
            data.status.adc_valid = reader.ReadWordLE();

            for (auto& als : data.visible_light)
            {
                for (auto& x : als)
                {
                    x = reader.ReadWordLE();
                }
            }

            data.temperature.structure = reader.ReadWordLE();
            for (auto& panel : data.temperature.panels)
            {
                panel = reader.ReadWordLE();
            }

            data.parameters.gain = reader.ReadByte();
            data.parameters.itime = reader.ReadByte();

            for (auto& als : data.infrared)
            {
                for (auto& x : als)
                {
                    x = reader.ReadWordLE();
                }
            }

            return reader.Status() ? OperationStatus::OK : OperationStatus::I2CReadFailed;
        }
    }
}
