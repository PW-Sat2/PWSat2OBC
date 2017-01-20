#ifndef LIBS_ADCS_IMTQ_H_
#define LIBS_ADCS_IMTQ_H_

#include "i2c/i2c.h"
#include "imtq/imtq.h"

namespace adcs
{
    class Imtq final
    {
      public:
        Imtq(drivers::i2c::II2CBus& i2cbus) : driver{i2cbus}
        {
        }

        using SelfTestResult = devices::imtq::SelfTestResult;
        using Dipole = devices::imtq::Dipole;
        using MagnetometerMeasurement = devices::imtq::MagnetometerMeasurement;

        bool PerformSelfTest(devices::imtq::SelfTestResult& result);
        bool ISISBDotDetumbling(std::chrono::seconds duration);
        bool PWSatDetumbling(const Vector3<Dipole>& dipole, Vector3<MagnetometerMeasurement>& mgtmMeasurement);

        devices::imtq::ImtqDriver driver;
    };
}

#endif // LIBS_ADCS_IMTQ_H_
