#ifndef SRC_DEVICES_IMTQ_H_
#define SRC_DEVICES_IMTQ_H_

#pragma once

#include "IImtqDriver.hpp"
#include "base/os.h"
#include "error_counter/error_counter.hpp"
#include "fwd.hpp"
#include "i2c/i2c.h"

/**
 * @defgroup ImtqDriver Magnetorquers low level driver
 * @ingroup device_drivers
 *
 * @brief This module is a low level imtq module driver.
 *
 * This driver is responsible for
 *  - direct communication with the hardware,
 *  - data parsing and interpretation
 *
 *  @{
 */

namespace devices
{
    namespace imtq
    {
        /**
         * @brief I2C address of Imtq board. 7-bit notation.
         */
        constexpr std::uint8_t I2Cadress = 0x10;

        /**
         * @brief Low level driver for Imtq commanding.
         */
        class ImtqDriver final : public IImtqDriver
        {
          public:
            /**
             * Constructs new instance of Imtq low-level driver
             * @param[in] errors Error counting mechanism
             * @param[in] i2cbus I2C bus used to communicate with device
             */
            ImtqDriver(error_counter::ErrorCounting& errors, drivers::i2c::II2CBus& i2cbus);

            // ----- Higher-level commands -----

            /**
             * Performs self test
             * @param[out] result Self test result
             * @param[in] tryToFixIsisErrors Whether to enable the alternative self-test algorithm.
             * @return Operation status.
             * This method runs:
             * 1) starts all axis self-test (TC-OP-08)
             * 2) waits specified amount of time for command to complete
             * 3) reads self test result (TC-DR-07)
             */
            virtual bool PerformSelfTest(SelfTestResult& result, bool tryToFixIsisErrors) override;

            /**
             * Measures magnetometer, having in mind possible on-going actuation.
             * @param[out] result Three axis magnetometer measurement
             * @return Operation status.
             * This method runs:
             * 1) cancels on-going actuation (TC-OP-03)
             * 2) waits wait magnetic field decay
             * 3) sends magnetometer measurement request (TC-OP-04)
             * 4) waits until read finishes
             * 3) reads magnetometer data (TC-DR-03)
             */
            virtual bool MeasureMagnetometer(Vector3<MagnetometerMeasurement>& result) override;

            // ----- Commands -----

            /**
             * @brief Performs SoftwareReset (TC-OP-01)
             * @return Operation status.
             */
            virtual bool SoftwareReset() override;

            /**
             * @brief Sends NoOperation command (TC-OP-02)
             * @return Operation status.
             */
            virtual bool SendNoOperation() override;

            /**
             * @brief Cancels current imtq operation (detumbling/self-test) (TC-OP-03)
             * @return Operation status.
             */
            virtual bool CancelOperation() override;

            /**
             * @brief Initiates magnetometer measurement (TC-OP-04)
             * @return Operation status.
             */
            virtual bool StartMTMMeasurement() override;

            /**
             * @brief Starts current-based actuation for specified time (TC-OP-05)
             * @param[in] current Three-axis current values for X, Y and Z axes.
             * @param[in] duration Duration of actuation, after which it will be cancelled automatically. 1ms resolution, maximum time 65
             * seconds.
             * @return Operation status.
             */
            virtual bool StartActuationCurrent(const Vector3<Current>& current, std::chrono::milliseconds duration) override;

            /**
             * @brief Starts dipole-based actuation for specified time (TC-OP-06)
             * @param[in] dipole Three-axis dipole values for X, Y and Z axes.
             * @param[in] duration Duration of actuation, after which it will be cancelled automatically. 1ms resolution, maximum time 65
             * seconds.
             * @return Operation status.
             */
            virtual bool StartActuationDipole(Vector3<Dipole> dipole, std::chrono::milliseconds duration) override;

            /**
             * @brief Starts self test (TC-OP-08)
             * @return Operation status.
             */
            virtual bool StartAllAxisSelfTest() override;

            /**
             * @brief Starts internal detumbling (TC-OP-09)
             * @param[in] duration Duration of detumbling. Resolution 1 second, maximum duration 65535 seconds.
             * @return Operation status.
             */
            virtual bool StartBDotDetumbling(std::chrono::seconds duration) override;

            // ----- Data requests -----
            /**
             * @brief Reads imtq State (TC-DR-01)
             * @param[out] state State structure read from imtq
             * @return Operation status.
             */
            virtual bool GetSystemState(State& state) override;

            /**
             * @brief Reads calibrated magnetometer data (TC-DR-03)
             * @param[out] result Magnetometer measurement
             * @return Operation status.
             * This command should be used after StartMTMMeasurement, having in mind magnetometer integration time.
             */
            virtual bool GetCalibratedMagnetometerData(MagnetometerMeasurementResult& result) override;

            /**
             * @brief Reads coil current (TC-DR-04)
             * @param[out] result Three-axis coil current measurement
             * @return Operation status.
             * This value is updated internally each 1ms.
             */
            virtual bool GetCoilCurrent(Vector3<Current>& result) override;

            /**
             * @brief Reads coil temperature (TC-DR-05)
             * @param[out] result Three-axis coil temperature measurement
             * @return Operation status.
             * This value is updated internally each 1ms.
             */
            virtual bool GetCoilTemperature(Vector3<TemperatureMeasurement>& result) override;

            /**
             * @brief Reads self-test result (TC-DR-07)
             * @param[out] result Self-test result
             * @return Operation status.
             * This command should be used after StartAllAxisSelfTest, having in mind self-test time.
             */
            virtual bool GetSelfTestResult(SelfTestResult& result) override;

            /**
             * @brief Reads imtq detumbling state (TC-DR-08)
             * @param[out] result Detumble data structure.
             * @return Operation status.
             * This value is updated internally each 1s.
             */
            virtual bool GetDetumbleData(DetumbleData& result) override;

            /**
             * @brief Reads HouseKeeping - RAW type (TC-DR-09)
             * @param[out] result RAW HouseKeeping read from imtq
             * @return Operation status.
             * This value is updated internally each 1ms.
             */
            virtual bool GetHouseKeepingRAW(HouseKeepingRAW& result) override;

            /**
             * @brief Reads HouseKeeping - Engineering type (TC-DR-10)
             * @param[out] result Engineering HouseKeeping read from imtq
             * @return Operation status.
             * This value is updated internally each 1ms.
             */
            virtual bool GetHouseKeepingEngineering(HouseKeepingEngineering& result) override;

            // ----- Configuration -----
            /**
             * @brief Parameter alias type
             */
            using Parameter = std::uint16_t;

            /**
             * @brief Reads specified parameter from imtq memory (TC-CF-01)
             * @param[in] id Parameter id
             * @param[out] result Parameter value. Should be span, with correct length, to memory to write result to.
             * @return Operation status.
             */
            virtual bool GetParameter(Parameter id, gsl::span<std::uint8_t> result) override;

            /**
             * @brief Sets specified parameter in imtq memory (TC-CF-02)
             * @param[in] id Parameter id
             * @param[in] value Parameter value. Should be span, with correct length, to memory containing parameter value.
             * @return Operation status.
             */
            virtual bool SetParameter(Parameter id, gsl::span<const std::uint8_t> value) override;

            /**
             * @brief Resets specified parameter in imtq memory to default one and reads it (TC-CF-03)
             * @param[in] id Parameter id
             * @param[out] result Parameter value. Should be span, with correct length, to memory to write result to.
             * @return Operation status.
             */
            virtual bool ResetParameterAndGetDefault(Parameter id, gsl::span<std::uint8_t> result);

            /**
             * @brief Last error occurred during last command execution.
             */
            ImtqDriverError LastError;

            /**
             * @brief Status byte returned from imtq during last command execution.
             * Defined in Imtq user manual, Table 3-3.
             */
            std::uint8_t LastStatus;

            /** @brief Error counter type */
            using ErrorCounter = error_counter::ErrorCounter<3>;

          private:
            /**
             * @brief Command opcodes as defined in Imtq documentation
             */
            enum class OpCode : std::uint8_t
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

            using ErrorReporter = error_counter::AggregatedErrorReporter<ErrorCounter::DeviceId>;

            ErrorCounter _error;
            drivers::i2c::II2CBus& i2cbus;

            bool CancelOperationInternal(error_counter::AggregatedErrorCounter& resultAggregator);
            bool StartMTMMeasurementInternal(error_counter::AggregatedErrorCounter& resultAggregator);
            bool GetCalibratedMagnetometerDataInternal(                 //
                MagnetometerMeasurementResult& result,                  //
                error_counter::AggregatedErrorCounter& resultAggregator //
                );
            bool StartAllAxisSelfTestInternal(error_counter::AggregatedErrorCounter& resultAggregator);
            bool GetSelfTestResultInternal( //
                SelfTestResult& result,     //
                error_counter::AggregatedErrorCounter& resultAggregator);

            bool GetOrResetParameter(                                   //
                OpCode opcode,                                          //
                Parameter id,                                           //
                gsl::span<std::uint8_t> result,                         //
                error_counter::AggregatedErrorCounter& resultAggregator //
                );

            bool SendCommandWithErrorHandling(OpCode opcode, gsl::span<const uint8_t> params = {});
            bool SendCommand(                                            //
                OpCode opcode,                                           //
                error_counter::AggregatedErrorCounter& resultAggregator, //
                gsl::span<const std::uint8_t> params = {}                //
                );

            bool DataRequestWithErrorHandling(OpCode opcode, gsl::span<uint8_t> response);
            bool DataRequest(                                           //
                OpCode opcode,                                          //
                gsl::span<std::uint8_t> response,                       //
                error_counter::AggregatedErrorCounter& resultAggregator //
                );

            bool WriteRead(OpCode opcode,
                gsl::span<const std::uint8_t> params,
                gsl::span<std::uint8_t> result,
                error_counter::AggregatedErrorCounter& resultAggregator);

            template <typename Result> bool GetHouseKeepingWithErrorHandling(OpCode opcode, Result& result);
        };
    }
}

/** @}*/

#endif /* SRC_DEVICES_IMTQ_H_ */
