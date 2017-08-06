#include <array>
#include <ostream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "experiment/suns/suns.hpp"
#include "mock/FsMock.hpp"
#include "rapidcheck.hpp"
#include "rapidcheck/gtest.h"

using namespace experiment::suns;
using namespace experiments::fs;
using namespace services::fs;
using std::hex;

namespace rc
{
    template <> struct Arbitrary<devices::suns::Status>
    {
        static auto arbitrary()
        {
            using T = devices::suns::Status;
            return gen::build<T>(       //
                gen::set(&T::ack),      //
                gen::set(&T::presence), //
                gen::set(&T::adc_valid) //
                );
        }
    };

    template <> struct Arbitrary<devices::suns::Params>
    {
        static auto arbitrary()
        {
            using T = devices::suns::Params;
            return gen::build<T>(   //
                gen::set(&T::gain), //
                gen::set(&T::itime) //
                );
        }
    };

    template <> struct Arbitrary<devices::suns::Temperatures>
    {
        static auto arbitrary()
        {
            using T = devices::suns::Temperatures;
            return gen::build<T>(        //
                gen::set(&T::structure), //
                gen::set(&T::panels)     //
                );
        }
    };

    template <> struct Arbitrary<devices::suns::MeasurementData>
    {
        static auto arbitrary()
        {
            using T = devices::suns::MeasurementData;

            return gen::build<T>(            //
                gen::set(&T::status),        //
                gen::set(&T::parameters),    //
                gen::set(&T::temperature),   //
                gen::set(&T::visible_light), //
                gen::set(&T::infrared)       //
                );
        }
    };

    template <> struct Arbitrary<devices::payload::PayloadTelemetry::SunsRef>
    {
        static auto arbitrary()
        {
            using T = devices::payload::PayloadTelemetry::SunsRef;

            return gen::build<T>(      //
                gen::set(&T::voltages) //
                );
        }
    };

    template <> struct Arbitrary<devices::gyro::GyroscopeTelemetry>
    {
        static auto arbitrary()
        {
            using T = devices::gyro::GyroscopeTelemetry;

            return gen::construct<T>(gen::arbitrary<std::int16_t>(),
                gen::arbitrary<std::int16_t>(),
                gen::arbitrary<std::int16_t>(),
                gen::arbitrary<std::int16_t>());
        }
    };

    template <> struct Arbitrary<DataPoint>
    {
        static auto arbitrary()
        {
            using T = DataPoint;
            return gen::build<T>(               //
                gen::set(&T::Timestamp),        //
                gen::set(&T::ExperimentalSunS), //
                gen::set(&T::ReferenceSunS),    //
                gen::set(&T::Gyro)              //
                );
        }
    };
}

namespace
{
    RC_GTEST_PROP(SunSDataPointTest, WritePrimaryDataSet, (DataPoint point))
    {
        testing::NiceMock<FsMock> fs;

        std::array<std::uint8_t, 1_KB> buffer;

        fs.AddFile("/suns", buffer);

        {
            ExperimentFile f;
            f.Open(fs, "/suns", FileOpen::CreateNew, FileAccess::WriteOnly);

            point.WritePrimaryDataSetTo(f);

            f.Close();
        }

        std::array<std::uint8_t, 200> expected;
        expected.fill(0xFF);
        Writer w(expected);
        w.WriteByte(num(ExperimentFile::PID::Synchronization));

        w.WriteByte(num(ExperimentFile::PID::Timestamp));
        w.WriteQuadWordLE(point.Timestamp.count());

        w.WriteByte(num(ExperimentFile::PID::ExperimentalSunSPrimary));
        w.WriteByte(0x11);
        w.WriteWordLE(point.ExperimentalSunS.status.ack);
        w.WriteWordLE(point.ExperimentalSunS.status.presence);
        w.WriteWordLE(point.ExperimentalSunS.status.adc_valid);
        for (auto v : point.ExperimentalSunS.visible_light)
        {
            for (auto y : v)
            {
                w.WriteWordLE(y);
            }
        }

        w.WriteWordLE(point.ExperimentalSunS.temperature.structure);
        w.WriteWordLE(point.ExperimentalSunS.temperature.panels[0]);
        w.WriteWordLE(point.ExperimentalSunS.temperature.panels[1]);
        w.WriteWordLE(point.ExperimentalSunS.temperature.panels[2]);

        w.WriteByte(num(ExperimentFile::PID::ReferenceSunS));
        for (auto v : point.ReferenceSunS.voltages)
        {
            w.WriteWordLE(v);
        }

        w.WriteByte(num(ExperimentFile::PID::Gyro));
        w.WriteWordLE(point.Gyro.X());
        w.WriteWordLE(point.Gyro.Y());
        w.WriteWordLE(point.Gyro.Z());
        w.WriteWordLE(point.Gyro.Temperature());

        for (auto i = 0U; i < expected.size(); i++)
        {
            if (expected[i] != buffer[i])
            {
                RC_LOG() << "Mismatch at " << i << ": 0x" << hex << (int)expected[i] << " != 0x" << hex << (int)buffer[i];
                RC_FAIL("not equal");
            }
        }
    }

    RC_GTEST_PROP(SunSDataPointTest, WriteSecondaryDataSet, (DataPoint point))
    {
        testing::NiceMock<FsMock> fs;

        std::array<std::uint8_t, 1_KB> buffer;

        fs.AddFile("/suns", buffer);

        {
            ExperimentFile f;
            f.Open(fs, "/suns", FileOpen::CreateNew, FileAccess::WriteOnly);

            point.WriteSecondaryDataSetTo(f);

            f.Close();
        }

        std::array<std::uint8_t, 200> expected;
        expected.fill(0xFF);
        Writer w(expected);
        w.WriteByte(num(ExperimentFile::PID::Synchronization));

        w.WriteByte(num(ExperimentFile::PID::Timestamp));
        w.WriteQuadWordLE(point.Timestamp.count());

        w.WriteByte(num(ExperimentFile::PID::ExperimentalSunSSecondary));
        w.WriteByte(point.ExperimentalSunS.parameters.gain);
        w.WriteByte(point.ExperimentalSunS.parameters.itime);
        for (auto v : point.ExperimentalSunS.infrared)
        {
            for (auto y : v)
            {
                w.WriteWordLE(y);
            }
        }
        for (auto i = 0U; i < expected.size(); i++)
        {
            if (expected[i] != buffer[i])
            {
                RC_LOG() << "Mismatch at " << i << ": 0x" << hex << (int)expected[i] << " != 0x" << hex << (int)buffer[i];
                RC_FAIL("not equal");
            }
        }
    }
}
