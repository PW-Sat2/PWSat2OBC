#ifndef SRC_DEVICES_IMTQ_H_
#define SRC_DEVICES_IMTQ_H_

#include <stdbool.h>
#include <array>
#include <cstdint>
#include <chrono>
#include <experimental/optional>
#include <gsl/span>

#include "base/os.h"
#include "i2c/i2c.h"

template<typename T>
using Vector3 = std::array<T, 3>;

namespace devices
{
    namespace imtq
    {
        constexpr std::uint8_t I2Cadress = 0x10;

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

        	Status();
        	Status(std::uint8_t);
        	bool IsNew();
        	bool InvalidX();
        	bool InvalidY();
        	bool InvalidZ();
        	Error CmdError();

        	std::uint8_t getValue() const;

          private:
        	std::uint8_t value;
        };

        // representation: 1 LSB = 1e-4 A
        using Current = std::uint16_t;

        // representation: 1 LSB = 1e-4 Am^2
        using Dipole = std::uint16_t;

    	enum class Mode : std::uint8_t
		{
    		Idle = 0,
			Selftest = 1,
			Detumble = 2
    	};

		class Error
		{
		 public:
			constexpr Error() : value{0} {};
			constexpr Error(std::uint8_t val) : value{val} {};

			bool Ok();
			bool I2CFailure();
			bool SPIFailure();
			bool ADCFailure();
			bool PWMFailure();
			bool SystemFailure();
			bool MagnetometerValuesOusideExpectedRange();
			bool CoilCurrentsOusideExpectedRange();

			std::uint8_t GetValue() const;
		 private:
			std::uint8_t value;
		};

        struct ImtqState
        {
			ImtqState() : status{},
			  			  mode{Mode::Idle},
						  error{0},
						  anyParameterUpdatedSinceStartup{false},
						  uptime{0} {}
        	Status status;
        	Mode mode;
        	Error error;
        	bool anyParameterUpdatedSinceStartup;
        	std::chrono::seconds uptime;
        };

        // representation: 1 LSB = 1e-7 T
        using MagnetometerMeasurement = std::uint32_t;

        struct MagnetometerMeasurementResult
        {
        	Vector3<MagnetometerMeasurement> data;
        	bool coilActuationDuringMeasurement;
        };

        // representation: 1 LSB = 1 centigrade
        using TemperatureMeasurement = std::uint16_t;

        struct SelfTestResult
        {
        	enum class Step
			{
				Init = 0x00, //
				Xp   = 0x01,
				Xn   = 0x02,
				Yp   = 0x03,
				Yn   = 0x04,
				Zp   = 0x05,
				Zn   = 0x06,
				Fina = 0x07
			};

        	struct StepResult
			{


				Error error;
				Step actualStep;

				Vector3<MagnetometerMeasurement> RawMagnetometerMeasurement;
				Vector3<MagnetometerMeasurement> CalibratedMagnetometerMeasurement;
				Vector3<Current> CoilCurrent;
				Vector3<TemperatureMeasurement> CoilTemperature;
			};

        	std::array<StepResult, 8> stepResults;
        };

        struct DetumbleData
        {
        	// representation -  1e-9 T/s
        	using BDotType = std::uint32_t;

        	Vector3<MagnetometerMeasurement> calibratedMagnetometerMeasurement;
        	Vector3<MagnetometerMeasurement> filteredMagnetometerMeasurement;
        	Vector3<BDotType> bDotData;
        	Vector3<Dipole> commandedDipole;
        	Vector3<Current> commandedCurrent;
        	Vector3<Current> measuredCurrent;
        };

        struct HouseKeepingRAW
        {
        	std::uint16_t digitalVoltage, analogVoltage;
        	std::uint16_t digitalCurrent, analogCurrent;
        	Vector3<std::uint16_t> coilCurrent;
        	Vector3<std::uint16_t> coilTemperature;
        	std::uint16_t MCUtemperature;
        };

        struct HouseKeepingEngineering
        {
        	using VoltageInMiliVolt = std::uint16_t;

        	VoltageInMiliVolt digitalVoltage, analogVoltage;
        	Current digitalCurrent, analogCurrent;
        	Vector3<VoltageInMiliVolt> coilCurrent;
        	Vector3<TemperatureMeasurement> coilTemperature;
        	TemperatureMeasurement MCUtemperature;
        };

        class ImtqDriver final
        {
          public:
            ImtqDriver(drivers::i2c::II2CBus& i2cbus);

            // ----- Commands -----
            bool SendNoOperation();
            bool SoftwareReset();
            bool CancelOperation();
            bool StartMTMMeasurement();
            bool StartActuationCurrent(Vector3<Current> current, std::chrono::milliseconds duration);
            bool StartActuationDipole(Vector3<Dipole> dipole, std::chrono::milliseconds duration);
            bool StartAllAxisSelfTest();
            bool StartBDotDetumbling(std::chrono::seconds duration);

            // ----- Data requests -----
            bool GetSystemState(ImtqState& state);
            bool GetCalibratedMagnetometerData(MagnetometerMeasurementResult& result);
            bool GetCoilCurrent(Vector3<Current>& result);
            bool GetCoilTemperature(Vector3<TemperatureMeasurement>& result);
            bool GetCommandedActuationDipole(Vector3<Dipole>& result);
            bool GetSelfTestResult(SelfTestResult& result);
            bool GetDetumbleData(DetumbleData& result);
            bool GetHouseKeepingRAW(HouseKeepingRAW& result);
            bool GetHouseKeepingEngineering(HouseKeepingEngineering& result);

            // ----- Configuration -----
            using Parameter = std::uint16_t;

            bool GetParameter(Parameter id, gsl::span<std::uint8_t> result);
            bool SetParameter(Parameter id, gsl::span<const std::uint8_t> value);
            bool ResetParameterAndGetDefault(Parameter id, gsl::span<std::uint8_t> result);

          private:
            drivers::i2c::II2CBus& i2cbus;

            bool SendCommand(OpCode opcode);
            bool SendCommand(OpCode opcode, gsl::span<const std::uint8_t> params);
            bool SendCommand(gsl::span<const std::uint8_t> params);
            bool DataRequest(OpCode opcode, gsl::span<std::uint8_t> response);
            bool GetParameterWithOpcode(OpCode opcode, Parameter id, gsl::span<std::uint8_t> result);
            bool WriteRead(OpCode opcode, gsl::span<const std::uint8_t> params, gsl::span<std::uint8_t> result);
        };
    }
}

#endif /* SRC_DEVICES_IMTQ_H_ */
