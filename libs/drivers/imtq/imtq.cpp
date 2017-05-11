/**
@file libs/drivers/imtq/imtq.cpp Driver for iMTQ module.

@remarks Based on ICD Issue 1.5 2015-07-22
*/
#include "imtq.h"
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
using error_counter::AggregatedErrorCounter;

using namespace std::chrono_literals;

namespace devices
{
    namespace imtq
    {
        // ------------------------- status -------------------------

        /**
         * @brief iMTQ status
         */
        class Status
        {
          public:
            /**
             * @brief Ctor
             * @param val Status value
             */
            Status(std::uint8_t val) : value{val}
            {
            }
            /**
             * @brief Is OK?
             * @return true if ok
             */
            bool IsOK() const
            {
                return ((value & 0b01111111) == 0);
            }

            /**
             * @brief Is Accepted?
             * @return true if accepted
             */
            bool Accepted() const
            {
                return (value & 0b1111) == 0;
            }
            /**
             * @brief Is new?
             * @return true if new
             */
            bool IsNew() const
            {
                return value & 0b10000000;
            }
            /**
             * @brief Is X invalid?
             * @return true if X is invalid
             */
            bool InvalidX() const
            {
                return value & 0b01000000;
            }
            /**
             * @brief Is Y invalid?
             * @return true if Y is invalid
             */
            bool InvalidY() const
            {
                return value & 0b00100000;
            }
            /**
             * @brief Is Z invalid?
             * @return true if Z is invalid
             */
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

        ImtqDriver::ImtqDriver(error_counter::ErrorCounting& errors, drivers::i2c::II2CBus& i2cbus)
            : _error(errors), //
              i2cbus{i2cbus}  //
        {
        }

        bool ImtqDriver::PerformSelfTest(SelfTestResult& result, bool tryToFixIsisErrors)
        {
            ErrorReporter errorContext(_error);

            // Disable actuations
            if (!CancelOperationInternal(errorContext.Counter()))
            {
                return false;
            }
            System::SleepTask(10ms);

            DetumbleData detumbleData;
            if (tryToFixIsisErrors)
            {
                // Measure reference magnetometer data
                if (!StartMTMMeasurementInternal(errorContext.Counter()))
                {
                    return false;
                }
                System::SleepTask(10ms);
                if (!GetDetumbleData(detumbleData))
                {
                    return false;
                }
            }

            // Do the self-test
            if (!StartAllAxisSelfTestInternal(errorContext.Counter()))
            {
                return false;
            }
            // 4s per axis + 1s for initialization + margin
            System::SleepTask(15s);
            if (!GetSelfTestResultInternal(result, errorContext.Counter()))
            {
                return false;
            }

            if (tryToFixIsisErrors)
            {
                // Reset errors on steps that give plausible results
                for (auto axis : {0, 1, 2})
                {
                    auto positiveStep = (int)SelfTestResult::Step::Xp + 2 * axis;
                    auto negativeStep = positiveStep + 1;

                    if (result.stepResults[positiveStep].CalibratedMagnetometerMeasurement[axis] >
                        detumbleData.calibratedMagnetometerMeasurement[axis])
                    {
                        result.stepResults[positiveStep].error = Error();
                    }
                    if (result.stepResults[negativeStep].CalibratedMagnetometerMeasurement[axis] <
                        detumbleData.calibratedMagnetometerMeasurement[axis])
                    {
                        result.stepResults[negativeStep].error = Error();
                    }
                }
            }

            return true;
        }

        bool ImtqDriver::MeasureMagnetometer(Vector3<MagnetometerMeasurement>& result)
        {
            ErrorReporter errorContext(_error);

            if (!CancelOperationInternal(errorContext.Counter()))
            {
                return false;
            }

            System::SleepTask(10ms); // magnetic field decay

            if (!StartMTMMeasurementInternal(errorContext.Counter()))
            {
                return false;
            }

            System::SleepTask(30ms); // integration time + margin

            MagnetometerMeasurementResult value;
            if (!GetCalibratedMagnetometerDataInternal(value, errorContext.Counter()))
            {
                return false;
            }

            result = value.data;

            return true;
        }

        // ----------------------------- Commands -----------------------------
        bool ImtqDriver::SendNoOperation()
        {
            return SendCommandWithErrorHandling(OpCode::NoOperation);
        }

        // TODO: re-think
        bool ImtqDriver::SoftwareReset()
        {
            ErrorReporter errorContext(_error);

            uint8_t opcode = static_cast<uint8_t>(OpCode::SoftwareReset);

            std::array<uint8_t, 2> response;

            auto result = i2cbus.Write(I2Cadress, gsl::span<const uint8_t, 1>(&opcode, 1));
            if (result != I2CResult::OK)
            {
                return false >> errorContext.Counter();
            }
            System::SleepTask(10ms);

            result = i2cbus.Read(I2Cadress, response);

            if (result == I2CResult::Nack)
            {
                return true;
            }
            if (result != I2CResult::OK)
            {
                return false >> errorContext.Counter();
            }
            if (response[0] == 0xFF && response[1] == 0xFF)
            {
                return true;
            }
            return false;
        }

        bool ImtqDriver::CancelOperation()
        {
            ErrorReporter errorContext(_error);
            return CancelOperationInternal(errorContext.Counter());
        }

        bool ImtqDriver::CancelOperationInternal(AggregatedErrorCounter& resultAggregator)
        {
            return SendCommand(OpCode::CancelOperation, resultAggregator);
        }

        bool ImtqDriver::StartMTMMeasurement()
        {
            ErrorReporter errorContext(_error);
            return StartMTMMeasurementInternal(errorContext.Counter());
        }

        bool ImtqDriver::StartMTMMeasurementInternal(AggregatedErrorCounter& resultAggregator)
        {
            return SendCommand(OpCode::StartMTMMeasurement, resultAggregator);
        }

        bool ImtqDriver::StartActuationCurrent(const Vector3<Current>& current, std::chrono::milliseconds duration)
        {
            std::array<uint8_t, 8> parameters;

            Writer writer{parameters};
            for (auto x : current)
            {
                writer.WriteSignedWordLE(x);
            }
            writer.WriteWordLE(duration.count());

            assert(writer.Status());
            return this->SendCommandWithErrorHandling(OpCode::StartActuationCurrent, writer.Capture());
        }

        bool ImtqDriver::StartActuationDipole(Vector3<Dipole> dipole, std::chrono::milliseconds duration)
        {
            std::array<uint8_t, 8> parameters;

            Writer writer{parameters};
            for (auto x : dipole)
            {
                writer.WriteWordLE(x);
            }
            writer.WriteSignedWordLE(duration.count());

            assert(writer.Status());
            return this->SendCommandWithErrorHandling(OpCode::StartActuationDipole, writer.Capture());
        }

        bool ImtqDriver::StartAllAxisSelfTest()
        {
            ErrorReporter errorContext(_error);
            return StartAllAxisSelfTestInternal(errorContext.Counter());
        }

        bool ImtqDriver::StartAllAxisSelfTestInternal(AggregatedErrorCounter& resultAggregator)
        {
            // 0x00 means test all axis test
            std::array<uint8_t, 1> parameters = {0x00};
            return this->SendCommand(OpCode::StartSelfTest, resultAggregator, parameters);
        }

        bool ImtqDriver::StartBDotDetumbling(std::chrono::seconds duration)
        {
            std::array<uint8_t, 2> parameters;

            Writer writer{parameters};
            writer.WriteWordLE(duration.count());

            assert(writer.Status());
            return this->SendCommandWithErrorHandling(OpCode::StartBDOT, writer.Capture());
        }

        // --------------------------- Data requests --------------------------

        bool ImtqDriver::GetSystemState(State& state)
        {
            std::array<uint8_t, 9> value;
            if (!this->DataRequestWithErrorHandling(OpCode::GetIMTQSystemState, value))
            {
                return false;
            }

            Reader reader{value};
            reader.Skip(2);
            state.mode = static_cast<Mode>(reader.ReadByte());
            state.error = Error{reader.ReadByte()};
            state.anyParameterUpdatedSinceStartup = (reader.ReadByte() == 1);
            state.uptime = std::chrono::seconds{reader.ReadDoubleWordLE()};

            assert(reader.Status());
            return true;
        }

        bool ImtqDriver::GetCalibratedMagnetometerData(MagnetometerMeasurementResult& result)
        {
            ErrorReporter errorContext(_error);
            return GetCalibratedMagnetometerDataInternal(result, errorContext.Counter());
        }

        bool ImtqDriver::GetCalibratedMagnetometerDataInternal( //
            MagnetometerMeasurementResult& result,              //
            AggregatedErrorCounter& resultAggregator            //
            )
        {
            std::array<uint8_t, 15> value;
            if (!this->DataRequest(OpCode::GetCalibratedMTMData, value, resultAggregator))
            {
                return false;
            }

            Reader reader{value};
            reader.Skip(2);
            for (auto& x : result.data)
            {
                x = reader.ReadSignedDoubleWordLE();
            }

            result.coilActuationDuringMeasurement = (reader.ReadByte() == 1);
            assert(reader.Status());
            return true;
        }

        bool ImtqDriver::GetCoilCurrent(Vector3<Current>& result)
        {
            std::array<uint8_t, 8> value;
            if (!this->DataRequestWithErrorHandling(OpCode::GetCoilCurrent, value))
            {
                return false;
            }

            Reader reader{value};
            reader.Skip(2);

            for (auto& x : result)
            {
                x = reader.ReadSignedWordLE();
            }
            assert(reader.Status());
            return true;
        }

        bool ImtqDriver::GetCoilTemperature(Vector3<TemperatureMeasurement>& result)
        {
            std::array<uint8_t, 8> value;
            if (!this->DataRequestWithErrorHandling(OpCode::GetCoilTemperatures, value))
            {
                return false;
            }

            Reader reader{value};
            reader.Skip(2);

            for (auto& x : result)
            {
                x = reader.ReadSignedWordLE();
            }
            assert(reader.Status());
            return true;
        }

        bool ImtqDriver::GetSelfTestResult(SelfTestResult& result)
        {
            ErrorReporter errorContext(_error);
            return GetSelfTestResultInternal(result, errorContext.Counter());
        }

        bool ImtqDriver::GetSelfTestResultInternal(SelfTestResult& result, AggregatedErrorCounter& resultAggregator)
        {
            std::array<uint8_t, 320> value;
            if (!this->DataRequest(OpCode::GetSelfTest, value, resultAggregator))
            {
                return false;
            }

            Reader reader{value};

            for (uint8_t step = 0; step < 8; ++step)
            {
                reader.Skip(2);
                result.stepResults[step].error = Error{reader.ReadByte()};
                result.stepResults[step].actualStep = static_cast<SelfTestResult::Step>(reader.ReadByte());
                for (auto& x : result.stepResults[step].RawMagnetometerMeasurement)
                {
                    x = reader.ReadSignedDoubleWordLE();
                }
                for (auto& x : result.stepResults[step].CalibratedMagnetometerMeasurement)
                {
                    x = reader.ReadSignedDoubleWordLE();
                }
                for (auto& x : result.stepResults[step].CoilCurrent)
                {
                    x = reader.ReadSignedWordLE();
                }
                for (auto& x : result.stepResults[step].CoilTemperature)
                {
                    x = reader.ReadSignedWordLE();
                }
            }
            assert(reader.Status());
            return true;
        }

        bool ImtqDriver::GetDetumbleData(DetumbleData& result)
        {
            std::array<uint8_t, 56> value;
            if (!this->DataRequestWithErrorHandling(OpCode::GetDetumbleData, value))
            {
                return false;
            }

            Reader reader{value};
            reader.Skip(2);

            for (auto& x : result.calibratedMagnetometerMeasurement)
            {
                x = reader.ReadSignedDoubleWordLE();
            }
            for (auto& x : result.filteredMagnetometerMeasurement)
            {
                x = reader.ReadSignedDoubleWordLE();
            }
            for (auto& x : result.bDotData)
            {
                x = reader.ReadSignedDoubleWordLE();
            }
            for (auto& x : result.commandedDipole)
            {
                x = reader.ReadSignedWordLE();
            }
            for (auto& x : result.commandedCurrent)
            {
                x = reader.ReadSignedWordLE();
            }
            for (auto& x : result.measuredCurrent)
            {
                x = reader.ReadSignedWordLE();
            }
            assert(reader.Status());
            return true;
        }

        bool ImtqDriver::GetHouseKeepingRAW(HouseKeepingRAW& result)
        {
            return GetHouseKeepingWithErrorHandling(OpCode::GetRAWHousekeepingData, result);
        }

        bool ImtqDriver::GetHouseKeepingEngineering(HouseKeepingEngineering& result)
        {
            return GetHouseKeepingWithErrorHandling(OpCode::GetEngineeringHousekeepingData, result);
        }

        // --------------------------- Configuration --------------------------

        bool ImtqDriver::GetParameter(Parameter id, gsl::span<std::uint8_t> result)
        {
            ErrorReporter errorContext(_error);
            return GetOrResetParameter(OpCode::GetParameter, id, result, errorContext.Counter());
        }

        bool ImtqDriver::SetParameter(Parameter id, gsl::span<const std::uint8_t> value)
        {
            ErrorReporter errorContext(_error);
            std::array<uint8_t, 10> paramsArray;

            Writer writer{paramsArray};
            writer.WriteWordLE(id);
            writer.WriteArray(value);

            span<uint8_t> params = writer.Capture();

            std::array<uint8_t, 12> responseArray;
            span<uint8_t> response{responseArray.begin(), 4 + value.size()};

            assert(writer.Status());
            bool OK_communication = WriteRead(OpCode::SetParameter, params, response, errorContext.Counter());
            bool OK_equalReturned = std::equal(params.begin(), params.end(), response.begin() + 2);
            return OK_communication && OK_equalReturned;
        }

        bool ImtqDriver::ResetParameterAndGetDefault(Parameter id, gsl::span<uint8_t> result)
        {
            ErrorReporter errorContext(_error);
            return GetOrResetParameter(OpCode::ResetParameter, id, result, errorContext.Counter());
        }

        // ------------------------- Private -------------------------

        bool ImtqDriver::GetOrResetParameter(        //
            OpCode opcode,                           //
            Parameter id,                            //
            gsl::span<uint8_t> result,               //
            AggregatedErrorCounter& resultAggregator //
            )
        {
            std::array<uint8_t, 2> request;

            Writer writer{request};
            writer.WriteWordLE(id);
            assert(writer.Status());

            std::array<uint8_t, 12> responseArray;
            span<uint8_t> response{responseArray.begin(), result.size() + 4};

            bool i2cError = WriteRead(opcode, request, response, resultAggregator);
            std::copy(response.begin() + 4, response.end(), result.begin());
            return i2cError;
        }

        bool ImtqDriver::SendCommandWithErrorHandling(OpCode opcode, span<const uint8_t> params)
        {
            ErrorReporter errorContext(_error);
            std::array<uint8_t, 2> responseArray;
            return WriteRead(opcode, params, responseArray, errorContext.Counter());
        }

        bool ImtqDriver::SendCommand(                 //
            OpCode opcode,                            //
            AggregatedErrorCounter& resultAggregator, //
            span<const uint8_t> params                //
            )
        {
            std::array<uint8_t, 2> responseArray;
            return WriteRead(opcode, params, responseArray, resultAggregator);
        }

        bool ImtqDriver::DataRequestWithErrorHandling(OpCode opcode, span<uint8_t> response)
        {
            ErrorReporter errorContext(_error);
            return DataRequest(opcode, response, errorContext.Counter());
        }

        bool ImtqDriver::DataRequest(                //
            OpCode opcode,                           //
            span<uint8_t> response,                  //
            AggregatedErrorCounter& resultAggregator //
            )
        {
            return WriteRead(opcode, {}, response, resultAggregator);
        }

        bool ImtqDriver::WriteRead(                  //
            OpCode opcode,                           //
            span<const uint8_t> params,              //
            span<uint8_t> response,                  //
            AggregatedErrorCounter& resultAggregator //
            )
        {
            assert(params.size() <= 10);
            if (params.size() > 10)
            {
                return false;
            }
            assert(response.size() >= 2);
            if (response.size() < 2)
            {
                return false;
            }

            constexpr int maximumWriteLength = 11;
            const auto opcodeByte = static_cast<uint8_t>(opcode);

            std::array<uint8_t, maximumWriteLength> output;
            Writer outputWriter{output};
            outputWriter.WriteByte(opcodeByte);
            outputWriter.WriteArray(params);

            span<uint8_t> request = outputWriter.Capture();
            auto i2cstatusWrite = i2cbus.Write(I2Cadress, request);
            if (i2cstatusWrite != I2CResult::OK)
            {
                LastError = ImtqDriverError::I2CWriteFailed;
                return false >> resultAggregator;
            }

            System::SleepTask(10ms);

            auto i2cstatusRead = i2cbus.Read(I2Cadress, response);
            if (i2cstatusRead != I2CResult::OK)
            {
                LastError = ImtqDriverError::I2CReadFailed;
                return false >> resultAggregator;
            }

            this->LastStatus = response[1];
            auto status = Status{LastStatus};

            if (opcodeByte != response[0])
            {
                LastError = ImtqDriverError::WrongOpcodeInResponse;
                return false;
            }
            if (!status.IsOK())
            {
                LastError = ImtqDriverError::StatusError;
                return false;
            }
            LastError = ImtqDriverError::OK;
            return true;
        }

        template <typename Result> bool ImtqDriver::GetHouseKeepingWithErrorHandling(OpCode opcode, Result& result)
        {
            ErrorReporter errorContext(_error);

            std::array<uint8_t, 24> value;
            bool i2cError = this->DataRequest(opcode, value, errorContext.Counter());

            Reader reader{value};
            reader.Skip(2);

            result.digitalVoltage = reader.ReadWordLE();
            result.analogVoltage = reader.ReadWordLE();
            result.digitalCurrent = reader.ReadWordLE();
            result.analogCurrent = reader.ReadWordLE();
            for (auto& x : result.coilCurrent)
            {
                x = reader.ReadWordLE();
            }
            for (auto& x : result.coilTemperature)
            {
                x = reader.ReadWordLE();
            }
            result.MCUtemperature = reader.ReadWordLE();
            assert(reader.Status());
            return i2cError;
        }
    }
}
