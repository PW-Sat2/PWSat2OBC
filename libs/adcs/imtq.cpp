#include "imtq.h"

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using gsl::span;
using drivers::i2c::I2CResult;
using namespace std::chrono_literals;

using namespace devices::imtq;

namespace adcs
{
    bool Imtq::PerformSelfTest(SelfTestResult& result)
    {
        ImtqState state;
        if (!driver.GetSystemState(state))
        {
            return false;
        }
        if (state.mode != Mode::Idle)
        {
            if (!driver.CancelOperation())
            {
                return false;
            }
        }

        System::SleepTask(100ms);

        if (!driver.StartAllAxisSelfTest())
        {
            return false;
        }
        // 8 times 3-axis measurement, default integration time 10ms
        System::SleepTask(240ms);

        for (int tries = 0; tries < 10; ++tries)
        {
            if (!driver.GetSystemState(state))
            {
                return false;
            }
            if (state.mode == Mode::Idle)
            {
                break;
            }
            System::SleepTask(50ms);
        }
        if (state.mode != Mode::Idle)
        {
            driver.CancelOperation();
            return false;
        }

        return driver.GetSelfTestResult(result);
    }

    bool Imtq::ISISBDotDetumbling(std::chrono::seconds duration)
    {
        ImtqState state;
        if (!driver.GetSystemState(state))
        {
            return false;
        }
        if (state.mode != Mode::Detumble)
        {
            if (!driver.CancelOperation())
            {
                return false;
            }
        }
        return driver.StartBDotDetumbling(duration);
    }

    bool Imtq::PWSatDetumbling(const Vector3<Dipole>& dipole, Vector3<MagnetometerMeasurement>& mgtmMeasurement)
    {
        if (!driver.CancelOperation())
        {
            return false;
        }

        System::SleepTask(10ms); // magnetic field decay

        driver.StartMTMMeasurement();

        System::SleepTask(30ms); // integration time

        bool timeout = true;
        for (int tries = 0; tries < 10; ++tries)
        {
            MagnetometerMeasurementResult result;
            bool newData;
            driver.GetCalibratedMagnetometerData(result, newData);
            if (result.coilActuationDuringMeasurement)
            {
                return false;
            }
            if (newData)
            {
                mgtmMeasurement = result.data;
                timeout = false;
                break;
            }
            System::SleepTask(10ms);
        }

        if (timeout)
        {
            return false;
        }

        return driver.StartActuationDipole(dipole, 1s);
    }
}
