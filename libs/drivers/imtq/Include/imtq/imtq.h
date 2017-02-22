#ifndef SRC_DEVICES_IMTQ_H_
#define SRC_DEVICES_IMTQ_H_

#include <stdbool.h>
#include <array>
#include <chrono>
#include <cstdint>
#include <gsl/span>

#include "base/os.h"
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
         * @brief Array of 3 elements.
         */
        template <typename T> using Vector3 = std::array<T, 3>;

        /**
         * @brief Current as returned by imtq. 1 LSB = 1e-4 A
         */
        using Current = std::int16_t;

        /**
         * @brief Magnetic dipole as returned by imtq. 1 LSB = 1e-4 Am^2
         */
        using Dipole = std::int16_t;

        /**
         * @brief Magnetic field measurement as returned by imtq. 1 LSB = 1e-7 T
         */
        using MagnetometerMeasurement = std::int32_t;

        /**
         * @brief Structure containing Three-axis magnetometer measurement.
         */
        struct MagnetometerMeasurementResult
        {
            /**
             * @brief Magnetic field measurement.
             */
            Vector3<MagnetometerMeasurement> data;

            /**
             * @brief true if coil were enabled during measurement
             */
            bool coilActuationDuringMeasurement;
        };

        /**
         * @brief Temperature as returned by imtq. 1 LSB = 1 centigrade
         */
        using TemperatureMeasurement = std::int16_t;

        /**
         * @brief Imtq internal mode
         */
        enum class Mode : std::uint8_t
        {
            /**
             * @brief MTM measurements and MTQ actuation are available upon request.
             * The user controls the subsystem via I2C commands.
             */
            Idle = 0,

            /**
             * @brief Actuate the MTQ in sequence and take measurements from all the sensors.
             */
            Selftest = 1,

            /**
             * @brief The BDOT algorithm is performed based on the MTM measurements and actuates the MTQ.
             * The state of the controller is available on request at any time.
             */
            Detumble = 2
        };

        /**
         * @brief Imtq internal error structure used during self-test.
         */
        class Error
        {
          public:
            /**
             * @brief Default constructor (no-error).
             */
            constexpr Error() : value{0}
            {
            }

            /**
             * @brief Structure containing Three-axis magnetometer measurement.
             */
            constexpr Error(std::uint8_t val) : value{val}
            {
            }

            /**
             * @brief Returns true if no error was detected.
             */
            bool Ok() const;

            /**
             * @brief Returns error code as defined in User Manual (TC-DR-07)
             */
            std::uint8_t GetValue() const;

          private:
            std::uint8_t value;
        };

        /**
         * @brief Imtq internal state structure used during GetSystemState command.
         */
        struct State
        {
            State() : mode{Mode::Idle}, error{0}, anyParameterUpdatedSinceStartup{false}, uptime{0}
            {
            }

            /**
             * @brief Imtq mode
             */
            Mode mode;

            /**
             * @brief Internal error occured during imtq operations
             */
            Error error;

            /**
             * @brief True if any configuration parameter was updated since start-up (via SetParameter command)
             */
            bool anyParameterUpdatedSinceStartup;

            /**
             * @brief Imtq uptime, with one second resolution
             */
            std::chrono::seconds uptime;
        };

        /**
         * @brief SelfTestResult structure returned during GetSelfTestResult.
         */
        struct SelfTestResult
        {
            /**
             * @brief Self test steps, each step should be performed (one after another in table) [TC-DR-07]
             */
            enum class Step
            {
                Init = 0x00, //
                Xp = 0x01,
                Xn = 0x02,
                Yp = 0x03,
                Yn = 0x04,
                Zp = 0x05,
                Zn = 0x06,
                Fina = 0x07
            };

            /**
             * @brief Each of self-test steps result is defined by following structure.
             */
            struct StepResult
            {
                /**
                 * @brief Actual step during self-test. Should be unique and increasing for each element.
                 */
                Step actualStep;

                /**
                 * @brief Error occurred during this step
                 */
                Error error;

                /**
                 * @brief Three-axis raw magnetometer data during self-test actuation.
                 */
                Vector3<MagnetometerMeasurement> RawMagnetometerMeasurement;

                /**
                 * @brief Three-axis calibrated magnetometer data during self-test actuation.
                 */
                Vector3<MagnetometerMeasurement> CalibratedMagnetometerMeasurement;

                /**
                 * @brief Three-axis coil current during self-test actuation.
                 */
                Vector3<Current> CoilCurrent;

                /**
                 * @brief Three-axis coil temperature during self-test actuation.
                 */
                Vector3<TemperatureMeasurement> CoilTemperature;
            };

            /**
             * @brief Array containing eight self-test steps.
             */
            std::array<StepResult, 8> stepResults;
        };

        /**
         * @brief Structure containing detumbling data (updated internally each iteration - nominal 1 Hz).
         */
        struct DetumbleData
        {
            /**
             * @brief BDot algorithm value: representation - 1e-9 T/s
             */
            using BDotType = std::int32_t;

            /**
             * @brief Three-axis calibrated magnetometer data during actual detumbling step.
             */
            Vector3<MagnetometerMeasurement> calibratedMagnetometerMeasurement;

            /**
             * @brief Three-axis filtered magnetometer data during actual detumbling step.
             */
            Vector3<MagnetometerMeasurement> filteredMagnetometerMeasurement;

            /**
             * @brief Three-axis BDot algorithm internal value during self-test actuation.
             */
            Vector3<BDotType> bDotData;

            /**
             * @brief Three-axis commanded dipole during self-test actuation.
             */
            Vector3<Dipole> commandedDipole;

            /**
             * @brief Three-axis commanded coil current during self-test actuation.
             */
            Vector3<Current> commandedCurrent;

            /**
             * @brief Three-axis measured coil current during self-test actuation.
             */
            Vector3<Current> measuredCurrent;
        };

        /**
         * @brief RAW housekeeping data structure. Conversion formulas are in TC-DR-09.
         */
        struct HouseKeepingRAW
        {
            /**
             * @brief Digital and analog supply voltages
             */
            std::uint16_t digitalVoltage, analogVoltage;

            /**
             * @brief Digital and analog supply currents
             */
            std::uint16_t digitalCurrent, analogCurrent;

            /**
             * @brief Three-axis coil current measurement
             */
            Vector3<std::uint16_t> coilCurrent;

            /**
             * @brief Three-axis coil temperature measurement
             */
            Vector3<std::uint16_t> coilTemperature;

            /**
             * @brief Measured microcontroller temperature.
             */
            std::uint16_t MCUtemperature;
        };

        /**
         * @brief Engineering notation housekeeping data structure. Conversion formulas are in TC-DR-09.
         */
        struct HouseKeepingEngineering
        {
            /**
             * @brief Voltage representation - 1 LSB = 1 mV
             */
            using VoltageInMiliVolt = std::uint16_t;

            /**
             * @brief Digital and analog supply voltages.
             */
            VoltageInMiliVolt digitalVoltage, analogVoltage;

            /**
             * @brief Digital and analog supply currents
             */
            Current digitalCurrent, analogCurrent;

            /**
             * @brief Three-axis coil current measurement
             */
            Vector3<Current> coilCurrent;

            /**
             * @brief Three-axis coil temperature measurement
             */
            Vector3<TemperatureMeasurement> coilTemperature;

            /**
             * @brief Measured microcontroller temperature.
             */
            TemperatureMeasurement MCUtemperature;
        };

        /**
         * @brief Low level driver for Imtq commanding.
         */
        class ImtqDriver final
        {
          public:
            /**
             * Constructs new instance of Imtq low-level driver
             * @param[in] i2cbus I2C bus used to communicate with device
             */
            ImtqDriver(drivers::i2c::II2CBus& i2cbus);

            // ----- Higher-level commands -----

            /**
             * Performs self test
             * @param[out] result Self test result
             * @return Operation status.
             * This method runs:
             * 1) starts all axis self-test (TC-OP-08)
             * 2) waits specified amount of time for command to complete
             * 3) reads self test result (TC-DR-07)
             */
            bool PerformSelfTest(SelfTestResult& result);

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
            bool MeasureMagnetometer(Vector3<MagnetometerMeasurement>& result);

            // ----- Commands -----

            /**
             * @brief Performs SoftwareReset (TC-OP-01)
             * @return Operation status.
             */
            bool SoftwareReset();

            /**
             * @brief Sends NoOperation command (TC-OP-02)
             * @return Operation status.
             */
            bool SendNoOperation();

            /**
             * @brief Cancels current imtq operation (detumbling/self-test) (TC-OP-03)
             * @return Operation status.
             */
            bool CancelOperation();

            /**
             * @brief Initiates magnetometer measurement (TC-OP-04)
             * @return Operation status.
             */
            bool StartMTMMeasurement();

            /**
             * @brief Starts current-based actuation for specified time (TC-OP-05)
             * @param[in] current Three-axis current values for X, Y and Z axes.
             * @param[in] duration Duration of actuation, after which it will be cancelled automatically. 1ms resolution, maximum time 65
             * seconds.
             * @return Operation status.
             */
            bool StartActuationCurrent(const Vector3<Current>& current, std::chrono::milliseconds duration);

            /**
             * @brief Starts dipole-based actuation for specified time (TC-OP-06)
             * @param[in] dipole Three-axis dipole values for X, Y and Z axes.
             * @param[in] duration Duration of actuation, after which it will be cancelled automatically. 1ms resolution, maximum time 65
             * seconds.
             * @return Operation status.
             */
            bool StartActuationDipole(Vector3<Dipole> dipole, std::chrono::milliseconds duration);

            /**
             * @brief Starts self test (TC-OP-08)
             * @return Operation status.
             */
            bool StartAllAxisSelfTest();

            /**
             * @brief Starts internal detumbling (TC-OP-09)
             * @param[in] duration Duration of detumbling. Resolution 1 second, maximum duration 65535 seconds.
             * @return Operation status.
             */
            bool StartBDotDetumbling(std::chrono::seconds duration);

            // ----- Data requests -----
            /**
             * @brief Reads imtq State (TC-DR-01)
             * @param[out] state State structure read from imtq
             * @return Operation status.
             */
            bool GetSystemState(State& state);

            /**
             * @brief Reads calibrated magnetometer data (TC-DR-03)
             * @param[out] result Magnetometer measurement
             * @return Operation status.
             * This command should be used after StartMTMMeasurement, having in mind magnetometer integration time.
             */
            bool GetCalibratedMagnetometerData(MagnetometerMeasurementResult& result);

            /**
             * @brief Reads coil current (TC-DR-04)
             * @param[out] result Three-axis coil current measurement
             * @return Operation status.
             * This value is updated internally each 1ms.
             */
            bool GetCoilCurrent(Vector3<Current>& result);

            /**
             * @brief Reads coil temperature (TC-DR-05)
             * @param[out] result Three-axis coil temperature measurement
             * @return Operation status.
             * This value is updated internally each 1ms.
             */
            bool GetCoilTemperature(Vector3<TemperatureMeasurement>& result);

            /**
             * @brief Reads self-test result (TC-DR-07)
             * @param[out] result Self-test result
             * @return Operation status.
             * This command should be used after StartAllAxisSelfTest, having in mind self-test time.
             */
            bool GetSelfTestResult(SelfTestResult& result);

            /**
             * @brief Reads imtq detumbling state (TC-DR-08)
             * @param[out] result Detumble data structure.
             * @return Operation status.
             * This value is updated internally each 1s.
             */
            bool GetDetumbleData(DetumbleData& result);

            /**
             * @brief Reads HouseKeeping - RAW type (TC-DR-09)
             * @param[out] result RAW HouseKeeping read from imtq
             * @return Operation status.
             * This value is updated internally each 1ms.
             */
            bool GetHouseKeepingRAW(HouseKeepingRAW& result);

            /**
             * @brief Reads HouseKeeping - Engineering type (TC-DR-10)
             * @param[out] result Engineering HouseKeeping read from imtq
             * @return Operation status.
             * This value is updated internally each 1ms.
             */
            bool GetHouseKeepingEngineering(HouseKeepingEngineering& result);

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
            bool GetParameter(Parameter id, gsl::span<std::uint8_t> result);

            /**
             * @brief Sets specified parameter in imtq memory (TC-CF-02)
             * @param[in] id Parameter id
             * @param[in] value Parameter value. Should be span, with correct length, to memory containing parameter value.
             * @return Operation status.
             */
            bool SetParameter(Parameter id, gsl::span<const std::uint8_t> value);

            /**
             * @brief Resets specified parameter in imtq memory to default one and reads it (TC-CF-03)
             * @param[in] id Parameter id
             * @param[out] result Parameter value. Should be span, with correct length, to memory to write result to.
             * @return Operation status.
             */
            bool ResetParameterAndGetDefault(Parameter id, gsl::span<std::uint8_t> result);

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

            drivers::i2c::II2CBus& i2cbus;

            bool GetOrResetParameter(OpCode opcode, Parameter id, gsl::span<std::uint8_t> result);

            bool SendCommand(OpCode opcode, gsl::span<const std::uint8_t> params = {});
            bool DataRequest(OpCode opcode, gsl::span<std::uint8_t> response);
            bool WriteRead(OpCode opcode, gsl::span<const std::uint8_t> params, gsl::span<std::uint8_t> result);
        };
    }
}

/** @}*/

#endif /* SRC_DEVICES_IMTQ_H_ */
