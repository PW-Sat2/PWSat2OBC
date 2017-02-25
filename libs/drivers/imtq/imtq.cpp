/**
@file libs/drivers/imtq/imtq.cpp Driver for iMTQ module.

@remarks Based on ICD Issue 1.5 2015-07-22
*/
#include "imtq.h"
#include <cassert>
#include <stdnoreturn.h>
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
using namespace std::chrono_literals;

namespace devices
{
    namespace imtq
    {
        // ------------------------- status -------------------------

        class Status
        {
          public:
            Status(std::uint8_t val) : value{val}
            {
            }
            bool IsOK() const
            {
                return ((value & 0b01111111) == 0);
            }
            bool Accepted() const
            {
                return (value & 0b1111) == 0;
            }
            bool IsNew() const
            {
                return value & 0b10000000;
            }
            bool InvalidX() const
            {
                return value & 0b01000000;
            }
            bool InvalidY() const
            {
                return value & 0b00100000;
            }
            bool InvalidZ() const
            {
                return value & 0b00010000;
            }

          private:
            std::uint8_t value;
        };

        // ------------------------- Error -------------------------

        bool Error::Ok() const
        {
            return (this->value == 0);
        }

        std::uint8_t Error::GetValue() const
        {
            return this->value;
        }

        // ------------------------- Public functions -------------------------

        ImtqDriver::ImtqDriver(drivers::i2c::II2CBus& i2cbus) : i2cbus{i2cbus}
        {
        }

        bool ImtqDriver::PerformSelfTest(SelfTestResult& result)
        {
            if (!StartAllAxisSelfTest())
            {
                return false;
            }
            // 8 times 3-axis measurement, default integration time 10ms + margin
            System::SleepTask(500ms);

            return GetSelfTestResult(result);
        }

        bool ImtqDriver::MeasureMagnetometer(Vector3<MagnetometerMeasurement>& result)
        {
            if (!CancelOperation())
            {
                return false;
            }

            System::SleepTask(10ms); // magnetic field decay

            if (!StartMTMMeasurement())
            {
                return false;
            }

            System::SleepTask(30ms); // integration time + margin

            MagnetometerMeasurementResult value;
            if (!GetCalibratedMagnetometerData(value))
            {
                return false;
            }

            result = value.data;

            return true;
        }

        // ----------------------------- Commands -----------------------------
        bool ImtqDriver::SendNoOperation()
        {
            return SendCommand(OpCode::NoOperation);
        }

        // TODO: re-think
        bool ImtqDriver::SoftwareReset()
        {
            uint8_t opcode = static_cast<uint8_t>(OpCode::SoftwareReset);

            std::array<uint8_t, 2> response;

            auto result = i2cbus.Write(I2Cadress, gsl::span<const uint8_t, 1>(&opcode, 1));
            if (result != I2CResult::OK)
            {
                return false;
            }
            System::SleepTask(10ms);

            result = i2cbus.Read(I2Cadress, response);

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

        bool ImtqDriver::StartActuationCurrent(const Vector3<Current>& current, std::chrono::milliseconds duration)
        {
            std::array<uint8_t, 8> parameters;

            Writer writer{parameters};
            for(auto x: current)
            {
                writer.WriteSignedWordLE(x);
            }
            writer.WriteWordLE(duration.count());

            return this->SendCommand(OpCode::StartActuationCurrent, writer.Capture());
        }

        bool ImtqDriver::StartActuationDipole(Vector3<Dipole> dipole, std::chrono::milliseconds duration)
        {
            std::array<uint8_t, 8> parameters;

            Writer writer{parameters};
            for(auto x: dipole)
            {
                writer.WriteWordLE(x);
            }
            writer.WriteSignedWordLE(duration.count());

            return this->SendCommand(OpCode::StartActuationDipole, writer.Capture());
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

            Writer writer{parameters};
            writer.WriteWordLE(duration.count());

            return this->SendCommand(OpCode::StartBDOT, writer.Capture());
        }

        // --------------------------- Data requests --------------------------

        bool ImtqDriver::GetSystemState(State& state)
        {
            std::array<uint8_t, 9> value;
            bool i2cError = this->DataRequest(OpCode::GetIMTQSystemState, value);

            Reader reader{value};
            reader.Skip(2);
            state.mode = static_cast<Mode>(reader.ReadByte());
            state.error = Error{reader.ReadByte()};
            state.anyParameterUpdatedSinceStartup = (reader.ReadByte() == 1);
            state.uptime = std::chrono::seconds{reader.ReadDoubleWordLE()};

            return i2cError;
        }

        bool ImtqDriver::GetCalibratedMagnetometerData(MagnetometerMeasurementResult& result)
        {
            std::array<uint8_t, 15> value;
            bool i2cError = this->DataRequest(OpCode::GetCalibratedMTMData, value);

            Reader reader{value};
            reader.Skip(2);
            for(auto& x: result.data)
            {
                x = reader.ReadSignedDoubleWordLE();
            }

            result.coilActuationDuringMeasurement = (reader.ReadByte() == 1);
            return i2cError;
        }

        bool ImtqDriver::GetCoilCurrent(Vector3<Current>& result)
        {
            std::array<uint8_t, 8> value;
            bool i2cError = this->DataRequest(OpCode::GetCoilCurrent, value);

            Reader reader{value};
            reader.Skip(2);

            for(auto& x: result)
            {
                x = reader.ReadSignedWordLE();
            }
            return i2cError;
        }

        bool ImtqDriver::GetCoilTemperature(Vector3<TemperatureMeasurement>& result)
        {
            std::array<uint8_t, 8> value;
            bool i2cError = this->DataRequest(OpCode::GetCoilTemperatures, value);

            Reader reader{value};
            reader.Skip(2);

            for(auto& x: result)
            {
                x = reader.ReadSignedWordLE();
            }
            return i2cError;
        }

        bool ImtqDriver::GetSelfTestResult(SelfTestResult& result)
        {
            std::array<uint8_t, 320> value;
            bool i2cError = this->DataRequest(OpCode::GetSelfTest, value);

            Reader reader{value};

            for (uint8_t step = 0; step < 8; ++step)
            {
                reader.Skip(2);
                result.stepResults[step].error = Error{reader.ReadByte()};
                result.stepResults[step].actualStep = static_cast<SelfTestResult::Step>(reader.ReadByte());
                for(auto& x: result.stepResults[step].RawMagnetometerMeasurement)
                {
                    x = reader.ReadSignedDoubleWordLE();
                }
                for(auto& x: result.stepResults[step].CalibratedMagnetometerMeasurement)
                {
                    x = reader.ReadSignedDoubleWordLE();
                }
                for(auto& x: result.stepResults[step].CoilCurrent)
                {
                    x = reader.ReadSignedWordLE();
                }
                for(auto& x: result.stepResults[step].CoilTemperature)
                {
                    x = reader.ReadSignedWordLE();
                }
            }
            return i2cError;
        }

        bool ImtqDriver::GetDetumbleData(DetumbleData& result)
        {
            std::array<uint8_t, 56> value;
            bool i2cError = this->DataRequest(OpCode::GetDetumbleData, value);

            Reader reader{value};
            reader.Skip(2);

            for(auto& x: result.calibratedMagnetometerMeasurement)
            {
                x = reader.ReadSignedDoubleWordLE();
            }
            for(auto& x: result.filteredMagnetometerMeasurement)
            {
                x = reader.ReadSignedDoubleWordLE();
            }
            for(auto& x: result.bDotData)
            {
                x = reader.ReadSignedDoubleWordLE();
            }
            for(auto& x: result.commandedDipole)
            {
                x = reader.ReadSignedWordLE();
            }
            for(auto& x: result.commandedCurrent)
            {
                x = reader.ReadSignedWordLE();
            }
            for(auto& x: result.measuredCurrent)
            {
                x = reader.ReadSignedWordLE();
            }
            return i2cError;
        }

        bool ImtqDriver::GetHouseKeepingRAW(HouseKeepingRAW& result)
        {
            return GetHouseKeeping(OpCode::GetRAWHousekeepingData, result);
        }

        bool ImtqDriver::GetHouseKeepingEngineering(HouseKeepingEngineering& result)
        {
            return GetHouseKeeping(OpCode::GetEngineeringHousekeepingData, result);
        }

        // --------------------------- Configuration --------------------------

        bool ImtqDriver::GetParameter(Parameter id, gsl::span<uint8_t> result)
        {
            return GetOrResetParameter(OpCode::GetParameter, id, result);
        }

        bool ImtqDriver::SetParameter(Parameter id, gsl::span<const uint8_t> value)
        {
            std::array<uint8_t, 10> paramsArray;

            Writer writer{paramsArray};
            writer.WriteWordLE(id);
            writer.WriteArray(value);

            span<uint8_t> params = writer.Capture();

            std::array<uint8_t, 12> responseArray;
            span<uint8_t> response{responseArray.begin(), 4 + value.size()};

            bool OK_communication = WriteRead(OpCode::SetParameter, params, response);
            bool OK_equalReturned = std::equal(params.begin(), params.end(), response.begin() + 2);
            return OK_communication && OK_equalReturned;
        }

        bool ImtqDriver::ResetParameterAndGetDefault(Parameter id, gsl::span<uint8_t> result)
        {
            return GetOrResetParameter(OpCode::ResetParameter, id, result);
        }

        // ------------------------- Private -------------------------

        bool ImtqDriver::GetOrResetParameter(OpCode opcode, Parameter id, gsl::span<uint8_t> result)
        {
            std::array<uint8_t, 2> request;

            Writer writer{request};
            writer.WriteWordLE(id);

            std::array<uint8_t, 12> responseArray;
            span<uint8_t> response{responseArray.begin(), result.size() + 4};

            bool i2cError = WriteRead(opcode, request, response);
            std::copy(response.begin() + 4, response.end(), result.begin());
            return i2cError;
        }

        bool ImtqDriver::SendCommand(OpCode opcode, span<const uint8_t> params)
        {
            std::array<uint8_t, 2> responseArray;
            return WriteRead(opcode, params, responseArray);
        }

        bool ImtqDriver::DataRequest(OpCode opcode, span<uint8_t> response)
        {
            return WriteRead(opcode, {}, response);
        }

        bool ImtqDriver::WriteRead(OpCode opcode, span<const uint8_t> params, span<uint8_t> response)
        {
            constexpr int maximumWriteLength = 11;
            assert(params.size() <= 10);
            const auto opcodeByte = static_cast<uint8_t>(opcode);

            std::array<uint8_t, maximumWriteLength> output;
            Writer outputWriter{output};
            outputWriter.WriteByte(opcodeByte);
            outputWriter.WriteArray(params);

            span<uint8_t> request = outputWriter.Capture();

            auto i2cstatusWrite = i2cbus.Write(I2Cadress, request);
            System::SleepTask(10ms);
            auto i2cstatusRead = i2cbus.Read(I2Cadress, response);

            this->LastStatus = response[1];
            auto status = Status{LastStatus};

            if (i2cstatusWrite != I2CResult::OK)
            {
                LastError = ImtqDriverError::I2CWriteFailed;
                return false;
            }
            if (i2cstatusRead != I2CResult::OK)
            {
                LastError = ImtqDriverError::I2CReadFailed;
                return false;
            }
            if (opcodeByte != response[0])
            {
                LastError = ImtqDriverError::WrongOpcodeInResponse;
                return false;
            }
            if (! status.IsOK())
            {
                LastError = ImtqDriverError::StatusError;
                return false;
            }
            LastError = ImtqDriverError::OK;
            return true;
        }

        template<typename Result>
        bool ImtqDriver::GetHouseKeeping(OpCode opcode, Result& result)
        {
            std::array<uint8_t, 24> value;
            bool i2cError = this->DataRequest(opcode, value);

            Reader reader{value};
            reader.Skip(2);

            result.digitalVoltage = reader.ReadWordLE();
            result.analogVoltage = reader.ReadWordLE();
            result.digitalCurrent = reader.ReadWordLE();
            result.analogCurrent = reader.ReadWordLE();
            for(auto& x: result.coilCurrent)
            {
                x = reader.ReadWordLE();
            }
            for(auto& x: result.coilTemperature)
            {
                x = reader.ReadWordLE();
            }
            result.MCUtemperature = reader.ReadWordLE();
            return i2cError;
        }
    }
}
