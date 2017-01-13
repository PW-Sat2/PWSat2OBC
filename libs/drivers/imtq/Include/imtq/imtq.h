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
//template<typename T>
//class Vector3 : public std::array<T, 3>
//{
//};

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

        	Status(std::uint8_t);
        	bool IsNew();
        	bool InvalidX();
        	bool InvalidY();
        	bool InvalidZ();
        	Error CmdError();

          private:
        	std::uint8_t value;
        };

        class CurrentMeasurement
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

        class Dipole
        {
          public:
        	std::uint16_t getIn0dot1miliAmpsPerMeterSq()
        	{
        		return value;
        	}
        	void setIn0dot1miliAmpsPerMeterSq(std::uint16_t value)
        	{
        		this->value = value;
        	}

          private:
        	// representation: 1 LSB = 1e-4 Am^2
        	std::uint16_t value;
        };

    	enum class Mode
		{
    		Idle = 0,
			Selftest = 1,
			Detumble = 2
    	};

        struct ImtqState
        {
        	Status status;
        	Mode mode;
        	bool error;
        	bool anyParameterUpdatedSinceStartup;
        	std::chrono::seconds uptime;
        };

        // representation: 1 LSB = 1e-7 T
        using MagnetometerMeasurement = std::uint32_t;

        struct MagnetometerMeasurementResult
        {
        	std::array<MagnetometerMeasurement, 3> data;
        	bool coilActuationDuringMeasurement;
        };

        // representation: 1 LSB = 1 centigrade
        using TemperatureMeasurement = std::uint16_t;

        struct SelfTestResult
        {
        	enum class SelfTestStep
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
				enum class Error
				{
					OK = 0x00,
					I2CFailure = 0x01,
					SPIFailure = 0x02,
					ADCFailure = 0x04,
					PWMFailure = 0x08,
					SystemFailure = 0x10,
					MagnetometerValuesOusideExpectedRange = 0x20,
					CoilCurrentsOusideExpectedRange = 0x40
				};

				Error error;
				SelfTestStep actualStep;

				Vector3<MagnetometerMeasurement> RawMagnetometerMeasurement;
				Vector3<MagnetometerMeasurement> CalibratedMagnetometerMeasurement;
				Vector3<CurrentMeasurement> CoilCurrent;
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
        	Vector3<CurrentMeasurement> commandedCurrent;
        	Vector3<CurrentMeasurement> measuredCurrent;
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
        	CurrentMeasurement digitalCurrent, analogCurrent;
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
            bool StartActuationCurrent(Vector3<CurrentMeasurement> current, std::chrono::milliseconds duration);
            bool StartActuationDipole(Vector3<Dipole> dipole, std::chrono::milliseconds duration);
            bool StartAllAxisSelfTest();
            bool StartBDotDetumbling(std::chrono::seconds duration);

            // ----- Data requests -----
            bool GetSystemState(ImtqState& state);
            bool GetCalibratedMagnetometerData(MagnetometerMeasurementResult& result);
            bool GetCoilCurrent(Vector3<CurrentMeasurement>& result);
            bool GetCoilTemperature(Vector3<TemperatureMeasurement>& result);
            bool GetCommandedActuationDipole(Vector3<Dipole>& result);
            bool GetSelfTestResult(SelfTestResult& result);
            bool GetDetumbleData(DetumbleData& result);
            bool GetHouseKeepingRAW(HouseKeepingRAW& result);
            bool GetHouseKeepingEngineering(HouseKeepingEngineering& result);

            // ----- Configuration -----
            using Parameter = std::uint16_t; //TODO: change to enum

            bool SetParameter(Parameter id, gsl::span<const uint8_t> value);
            bool GetParameter(Parameter id, gsl::span<uint8_t> result);
            bool ResetParameterAndGetDefault(Parameter id, gsl::span<uint8_t> result);

          private:
            drivers::i2c::II2CBus& i2cbus;

            bool SendCommand(OpCode opcode);
            bool SendCommand(OpCode opcode, gsl::span<const std::uint8_t> params);
            bool SendCommand(gsl::span<const std::uint8_t> params);
        };
    }
}

#endif /* SRC_DEVICES_IMTQ_H_ */
