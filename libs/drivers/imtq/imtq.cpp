/**
@file libs/drivers/imtq/imtq.cpp Driver for iMTQ module.

@remarks Based on ICD Issue 1.5 2015-07-22
*/
#include "imtq.h"
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
        namespace details
        {
            int16_t UnsignedToSignedWord(uint16_t value)
            {
                // return *reinterpret_cast<int16_t*>(&value);
                return (int16_t)(value);
            }
            int32_t UnsignedToSignedDoubleWord(uint32_t value)
            {
                return (int32_t)(value);
            }

            int16_t ReadSignedWordLE(Reader& reader)
            {
                return UnsignedToSignedWord(reader.ReadWordLE());
            }

            int32_t ReadSignedDoubleWordLE(Reader& reader)
            {
                return UnsignedToSignedDoubleWord(reader.ReadDoubleWordLE());
            }
        }
#define FOR_AXIS(var) for (uint8_t var = 0; var < 3; ++var)

        using namespace details;

        constexpr int maximumWriteLength = 11;

        // ------------------------- status -------------------------

        Status::Status() : value{0}
        {
        }

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

        uint8_t Status::getValue() const
        {
            return this->value;
        }

        // ------------------------- Error -------------------------
        //		Error::Error() : value{0}
        //		{
        //		}
        //
        //		Error::Error(std::uint8_t val) : value{val}
        //		{
        //		}

        bool Error::Ok()
        {
            return (this->value == 0);
        }

        bool Error::I2CFailure()
        {
            return (this->value & 0x01);
        }

        bool Error::SPIFailure()
        {
            return (this->value & 0x02);
        }

        bool Error::ADCFailure()
        {
            return (this->value & 0x04);
        }

        bool Error::PWMFailure()
        {
            return (this->value & 0x08);
        }

        bool Error::SystemFailure()
        {
            return (this->value & 0x10);
        }

        bool Error::MagnetometerValuesOusideExpectedRange()
        {
            return (this->value & 0x20);
        }

        bool Error::CoilCurrentsOusideExpectedRange()
        {
            return (this->value & 0x40);
        }

        uint8_t Error::GetValue() const
        {
            return value;
        }

        // ------------------------- Public functions -------------------------

        ImtqDriver::ImtqDriver(drivers::i2c::II2CBus& i2cbus) : i2cbus{i2cbus}
        {
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

            auto result = i2cbus.WriteRead(I2Cadress, gsl::span<const uint8_t, 1>(&opcode, 1), response);

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
            Writer writer;
            WriterInitialize(&writer, parameters.data(), parameters.size());
            FOR_AXIS(i)
            {
                WriterWriteSignedWordLE(&writer, current[i]);
            }
            WriterWriteWordLE(&writer, duration.count());

            return this->SendCommand(OpCode::StartActuationCurrent, parameters);
        }

        bool ImtqDriver::StartActuationDipole(Vector3<Dipole> dipole, std::chrono::milliseconds duration)
        {
            std::array<uint8_t, 8> parameters;
            Writer writer;
            WriterInitialize(&writer, parameters.data(), parameters.size());
            FOR_AXIS(i)
            {
                WriterWriteWordLE(&writer, dipole[i]);
            }
            WriterWriteSignedWordLE(&writer, duration.count());

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

        // --------------------------- Data requests --------------------------

        bool ImtqDriver::GetSystemState(ImtqState& state)
        {
            std::array<uint8_t, 9> value;
            if (!this->DataRequest(OpCode::GetIMTQSystemState, value))
            {
                return false;
            }

            state.status = static_cast<Status>(value[1]);
            state.mode = static_cast<Mode>(value[2]);
            state.error = static_cast<Error>(value[3]);
            state.anyParameterUpdatedSinceStartup = (value[4] == 1);

            Reader reader{span<uint8_t, 4>(&value[5], 4)};
            state.uptime = std::chrono::seconds{reader.ReadDoubleWordLE()};

            return true;
        }

        bool ImtqDriver::GetCalibratedMagnetometerData(MagnetometerMeasurementResult& result, bool& newValue)
        {
            std::array<uint8_t, 15> value;
            if (!this->DataRequest(OpCode::GetCalibratedMTMData, value))
            {
                return false;
            }
            newValue = Status{value[1]}.IsNew();

            Reader reader{value};
            reader.Skip(2);
            FOR_AXIS(i)
            {
                result.data[i] = ReadSignedDoubleWordLE(reader);
            }

            result.coilActuationDuringMeasurement = (value[14] == 1);
            return true;
        }

        bool ImtqDriver::GetCoilCurrent(Vector3<Current>& result)
        {
            std::array<uint8_t, 8> value;
            if (!this->DataRequest(OpCode::GetCoilCurrent, value))
            {
                return false;
            }

            Reader reader{value};
            reader.Skip(2);

            FOR_AXIS(i)
            {
                result[i] = ReadSignedWordLE(reader);
            }
            return true;
        }

        bool ImtqDriver::GetCoilTemperature(Vector3<TemperatureMeasurement>& result)
        {
            std::array<uint8_t, 8> value;
            if (!this->DataRequest(OpCode::GetCoilTemperatures, value))
            {
                return false;
            }

            Reader reader{value};
            reader.Skip(2);

            FOR_AXIS(i)
            {
                result[i] = ReadSignedWordLE(reader);
            }
            return true;
        }

        bool ImtqDriver::GetCommandedActuationDipole(Vector3<Dipole>& result)
        {
            std::array<uint8_t, 8> value;
            if (!this->DataRequest(OpCode::GetCommandedActuationDipole, value))
            {
                return false;
            }

            Reader reader{value};
            reader.Skip(2);

            FOR_AXIS(i)
            {
                result[i] = ReadSignedWordLE(reader);
            }
            return true;
        }

        bool ImtqDriver::GetSelfTestResult(SelfTestResult& result)
        {
            std::array<uint8_t, 320> value;
            if (!this->DataRequest(OpCode::GetSelfTest, value))
            {
                return false;
            }

            Reader reader{value};

            for (uint8_t step = 0; step < 8; ++step)
            {
                reader.Skip(2);
                result.stepResults[step].error = static_cast<Error>(reader.ReadByte());
                result.stepResults[step].actualStep = static_cast<SelfTestResult::Step>(reader.ReadByte());
                FOR_AXIS(i)
                {
                    result.stepResults[step].RawMagnetometerMeasurement[i] = ReadSignedDoubleWordLE(reader);
                }
                FOR_AXIS(i)
                {
                    result.stepResults[step].CalibratedMagnetometerMeasurement[i] = ReadSignedDoubleWordLE(reader);
                }
                FOR_AXIS(i)
                {
                    result.stepResults[step].CoilCurrent[i] = ReadSignedWordLE(reader);
                }
                FOR_AXIS(i)
                {
                    result.stepResults[step].CoilTemperature[i] = ReadSignedWordLE(reader);
                }
            }
            return true;
        }

        bool ImtqDriver::GetDetumbleData(DetumbleData& result)
        {
            std::array<uint8_t, 56> value;
            if (!this->DataRequest(OpCode::GetDetumbleData, value))
            {
                return false;
            }

            Reader reader{value};
            reader.Skip(2);

            FOR_AXIS(i)
            {
                result.calibratedMagnetometerMeasurement[i] = ReadSignedDoubleWordLE(reader);
            }
            FOR_AXIS(i)
            {
                result.filteredMagnetometerMeasurement[i] = ReadSignedDoubleWordLE(reader);
            }
            FOR_AXIS(i)
            {
                result.bDotData[i] = ReadSignedDoubleWordLE(reader);
            }
            FOR_AXIS(i)
            {
                result.commandedDipole[i] = ReadSignedWordLE(reader);
            }
            FOR_AXIS(i)
            {
                result.commandedCurrent[i] = ReadSignedWordLE(reader);
            }
            FOR_AXIS(i)
            {
                result.measuredCurrent[i] = ReadSignedWordLE(reader);
            }
            return true;
        }

        bool ImtqDriver::GetHouseKeepingRAW(HouseKeepingRAW& result)
        {
            std::array<uint8_t, 24> value;
            if (!this->DataRequest(OpCode::GetRAWHousekeepingData, value))
            {
                return false;
            }

            Reader reader{value};
            reader.Skip(2);

            result.digitalVoltage = reader.ReadWordLE();
            result.analogVoltage = reader.ReadWordLE();
            result.digitalCurrent = reader.ReadWordLE();
            result.analogCurrent = reader.ReadWordLE();
            FOR_AXIS(i)
            {
                result.coilCurrent[i] = reader.ReadWordLE();
            }
            FOR_AXIS(i)
            {
                result.coilTemperature[i] = reader.ReadWordLE();
            }
            result.MCUtemperature = reader.ReadWordLE();
            return true;
        }

        bool ImtqDriver::GetHouseKeepingEngineering(HouseKeepingEngineering& result)
        {
            std::array<uint8_t, 24> value;
            if (!this->DataRequest(OpCode::GetEngineeringHousekeepingData, value))
            {
                return false;
            }

            Reader reader{value};
            reader.Skip(2);

            result.digitalVoltage = reader.ReadWordLE();
            result.analogVoltage = reader.ReadWordLE();
            result.digitalCurrent = reader.ReadWordLE();
            result.analogCurrent = reader.ReadWordLE();
            FOR_AXIS(i)
            {
                result.coilCurrent[i] = ReadSignedWordLE(reader);
            }
            FOR_AXIS(i)
            {
                result.coilTemperature[i] = ReadSignedWordLE(reader);
            }
            result.MCUtemperature = ReadSignedWordLE(reader);
            return true;
        }

        // --------------------------- Configuration --------------------------

        bool ImtqDriver::GetParameter(Parameter id, gsl::span<uint8_t> result)
        {
            return GetParameterWithOpcode(OpCode::GetParameter, id, result);
        }

        bool ImtqDriver::SetParameter(Parameter id, gsl::span<const uint8_t> value)
        {
            if (value.size() > 8)
            {
                return false;
            }
            std::array<uint8_t, 10> paramsArray;
            Writer writer;
            WriterInitialize(&writer, paramsArray.begin(), 3);
            WriterWriteWordLE(&writer, id);
            std::copy(value.begin(), value.end(), paramsArray.begin() + writer.position);
            span<uint8_t> params{paramsArray.begin(), 2 + value.size()};

            std::array<uint8_t, 12> responseArray;
            span<uint8_t> response{responseArray.begin(), 4 + value.size()};

            if (!WriteRead(OpCode::SetParameter, params, response) || !std::equal(params.begin(), params.end(), response.begin() + 2))
            {
                return false;
            }
            return true;
        }

        bool ImtqDriver::ResetParameterAndGetDefault(Parameter id, gsl::span<uint8_t> result)
        {
            return GetParameterWithOpcode(OpCode::ResetParameter, id, result);
        }

        // ------------------------- Private -------------------------

        bool ImtqDriver::SendCommand(OpCode opcode)
        {
            return SendCommand(opcode, {});
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

        bool ImtqDriver::GetParameterWithOpcode(OpCode opcode, Parameter id, gsl::span<uint8_t> result)
        {
            if (result.size() > 8)
            {
                return false;
            }
            std::array<uint8_t, 2> params;
            Writer writer;
            WriterInitialize(&writer, params.begin(), 2);
            WriterWriteWordLE(&writer, id);

            std::array<uint8_t, 12> responseArray;
            span<uint8_t> response(responseArray.begin(), result.size() + 4);

            if (!WriteRead(opcode, params, response))
            {
                return false;
            }
            std::copy(response.begin() + 4, response.end(), result.begin());
            return true;
        }

        bool ImtqDriver::WriteRead(OpCode opcode, span<const uint8_t> params, span<uint8_t> response)
        {
            std::array<uint8_t, maximumWriteLength> output;
            output[0] = static_cast<uint8_t>(opcode);
            std::copy(params.begin(), params.end(), output.begin() + 1);
            span<uint8_t> request{output.begin(), params.size() + 1};

            auto i2cstatus = i2cbus.WriteRead(I2Cadress, request, response);

            auto status = Status{response[1]};
            if (i2cstatus != I2CResult::OK || static_cast<uint8_t>(opcode) != response[0] || status.CmdError() != Status::Error::Accepted ||
                status.InvalidX() || status.InvalidY() || status.InvalidZ())
            {
                return false;
            }
            return true;
        }

#undef FOR_AXIS
    }
}
