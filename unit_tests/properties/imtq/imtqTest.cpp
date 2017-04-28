#include <algorithm>
#include <em_i2c.h>
#include <gsl/span>
#include <string>
#include <tuple>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "I2C/I2CMock.hpp"
#include "OsMock.hpp"
#include "base/reader.h"
#include "base/writer.h"
#include "i2c/i2c.h"
#include "os/os.hpp"
#include "rapidcheck.hpp"
#include "rapidcheck/gtest.h"
#include "system.h"
#include "utils.hpp"

#include "imtq/imtq.h"

using testing::_;
using testing::Eq;
using testing::Ne;
using testing::Ge;
using testing::StrEq;
using testing::Return;
using testing::Invoke;
using testing::Pointee;
using testing::ElementsAre;
using testing::Matches;
using gsl::span;
using drivers::i2c::I2CResult;
using namespace devices::imtq;
namespace
{
    static const uint8_t ImtqAddress = 0x10;

    class ImtqTest : public testing::Test
    {
      public:
        ImtqTest() : imtq(i2c)
        {
        }

      protected:
        devices::imtq::ImtqDriver imtq;
        I2CBusMock i2c;
    };

    template <typename IMTQ, typename I2C, typename Method>
    void testActuation(IMTQ& imtq, I2C& i2c, Method method, uint8_t command, std::array<int16_t, 3> values, uint16_t duration)
    {
        EXPECT_CALL(i2c, Write(ImtqAddress, BeginsWith(command))).WillOnce(Invoke([&](uint8_t /*address*/, auto inData) {
            EXPECT_TRUE(inData.size() == 9);

            Reader reader{inData};
            reader.Skip(1);
            EXPECT_TRUE(reader.ReadSignedWordLE() == values[0]);
            EXPECT_TRUE(reader.ReadSignedWordLE() == values[1]);
            EXPECT_TRUE(reader.ReadSignedWordLE() == values[2]);

            EXPECT_TRUE(reader.ReadWordLE() == duration);

            return I2CResult::OK;
        }));

        EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Invoke([&](uint8_t /*address*/, auto outData) {
            EXPECT_TRUE(outData.size() == 2);

            outData[0] = command;
            outData[1] = 0;
            return I2CResult::OK;
        }));

        RC_ASSERT((imtq.*method)(values, std::chrono::milliseconds{duration}));
    }

    RC_GTEST_FIXTURE_PROP(ImtqTest, StartActuationCurrent, (std::array<int16_t, 3> currents, uint16_t duration))
    {
        testActuation(imtq, i2c, &devices::imtq::ImtqDriver::StartActuationCurrent, 0x05, currents, duration);
    }

    RC_GTEST_FIXTURE_PROP(ImtqTest, StartActuationDipole, (std::array<int16_t, 3> dipole, uint16_t duration))
    {
        testActuation(imtq, i2c, &devices::imtq::ImtqDriver::StartActuationDipole, 0x06, dipole, duration);
    }

    RC_GTEST_FIXTURE_PROP(ImtqTest, StartBDotDetumbling, (uint16_t duration))
    {
        EXPECT_CALL(i2c, Write(ImtqAddress, BeginsWith(0x09))).WillOnce(Invoke([&](uint8_t /*address*/, auto inData) {
            EXPECT_EQ(inData.size(), 3);

            Reader reader{inData};
            reader.Skip(1);
            EXPECT_EQ(reader.ReadWordLE(), duration);

            return I2CResult::OK;
        }));
        EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Invoke([&](uint8_t /*address*/, auto outData) {
            EXPECT_EQ(outData.size(), 2);

            outData[0] = 0x09;
            outData[1] = 0;
            return I2CResult::OK;
        }));

        RC_ASSERT(imtq.StartBDotDetumbling(std::chrono::seconds{duration}));
    }

    RC_GTEST_FIXTURE_PROP(ImtqTest, GetSystemState, ())
    {
        uint8_t mode = *rc::gen::inRange(0, 3);
        uint8_t error = *rc::gen::inRange(0, 128);
        uint8_t conf = *rc::gen::inRange(0, 2);
        uint32_t uptime = *rc::gen::arbitrary<int>();

        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0x41))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Invoke([&](uint8_t /*address*/, auto outData) {
            EXPECT_EQ(outData.size(), 9);

            Writer writer{outData};
            writer.WriteByte(0x41);
            writer.WriteByte(0);

            writer.WriteByte(mode);
            writer.WriteByte(error);
            writer.WriteByte(conf);
            writer.WriteDoubleWordLE(uptime);

            return I2CResult::OK;
        }));

        State state;
        RC_ASSERT(imtq.GetSystemState(state));

        RC_ASSERT(static_cast<uint8_t>(state.mode) == mode);
        RC_ASSERT(state.error.GetValue() == error);
        RC_ASSERT(state.anyParameterUpdatedSinceStartup == conf);
        RC_ASSERT(state.uptime == std::chrono::seconds{uptime});
    }

    RC_GTEST_FIXTURE_PROP(ImtqTest, GetCalibratedMagnetometerData, (std::array<int32_t, 3> data, bool coilAct))
    {
        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0x43))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Invoke([=](uint8_t /*address*/, auto outData) {
            EXPECT_EQ(outData.size(), 15);

            Writer writer{outData};
            writer.WriteByte(0x43);
            writer.WriteByte(0);

            writer.WriteSignedDoubleWordLE(data[0]);
            writer.WriteSignedDoubleWordLE(data[1]);
            writer.WriteSignedDoubleWordLE(data[2]);

            writer.WriteByte(coilAct);

            return I2CResult::OK;
        }));

        MagnetometerMeasurementResult mgtm;
        RC_ASSERT(imtq.GetCalibratedMagnetometerData(mgtm));

        RC_ASSERT(mgtm.coilActuationDuringMeasurement == coilAct);
        RC_ASSERT(mgtm.data == data);
    }

    template <typename IMTQ, typename I2C, typename Method>
    void testCurrentTempDipole(IMTQ& imtq, I2C& i2c, Method method, uint8_t command, std::array<int16_t, 3> data)
    {
        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(command))).WillOnce(Return(I2CResult::OK));

        EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Invoke([&](uint8_t /*address*/, auto outData) {
            EXPECT_TRUE(outData.size() == 8);

            Writer writer{outData};
            writer.WriteByte(command);
            writer.WriteByte(0x00);

            writer.WriteSignedWordLE(data[0]);
            writer.WriteSignedWordLE(data[1]);
            writer.WriteSignedWordLE(data[2]);
            return I2CResult::OK;
        }));

        Vector3<int16_t> result;
        RC_ASSERT((imtq.*method)(result));
        RC_ASSERT(result == data);
    }

    RC_GTEST_FIXTURE_PROP(ImtqTest, GetCoilCurrent, (std::array<int16_t, 3> data))
    {
        testCurrentTempDipole(imtq, i2c, &devices::imtq::ImtqDriver::GetCoilCurrent, 0x44, data);
    }

    RC_GTEST_FIXTURE_PROP(ImtqTest, GetCoilTemperature, (std::array<int16_t, 3> data))
    {
        testCurrentTempDipole(imtq, i2c, &devices::imtq::ImtqDriver::GetCoilTemperature, 0x45, data);
    }

    inline bool operator==(const SelfTestResult::StepResult a, const SelfTestResult::StepResult b)
    {
        if (a.error.GetValue() == b.error.GetValue() && a.actualStep == b.actualStep &&
            a.RawMagnetometerMeasurement == b.RawMagnetometerMeasurement &&
            a.CalibratedMagnetometerMeasurement == b.CalibratedMagnetometerMeasurement && a.CoilCurrent == b.CoilCurrent &&
            a.CoilTemperature == b.CoilTemperature)
            return true;
        return false;
    }

    RC_GTEST_FIXTURE_PROP(ImtqTest, GetSelfTestResult, ())
    {
        SelfTestResult data;

        for (int i = 0; i < 8; ++i)
        {
            uint8_t err = *rc::gen::inRange(0, 128);
            data.stepResults[i].error = Error{err};
            data.stepResults[i].actualStep = static_cast<SelfTestResult::Step>(i);

            data.stepResults[i].RawMagnetometerMeasurement = *rc::gen::arbitrary<Vector3<int32_t>>();
            data.stepResults[i].CalibratedMagnetometerMeasurement = *rc::gen::arbitrary<Vector3<int32_t>>();
            data.stepResults[i].CoilCurrent = *rc::gen::arbitrary<Vector3<int16_t>>();
            data.stepResults[i].CoilTemperature = *rc::gen::arbitrary<Vector3<int16_t>>();
        }
        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0x47))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Invoke([&](uint8_t /*address*/, auto outData) {
            EXPECT_TRUE(outData.size() == 320);

            Writer writer{outData};

            for (int i = 0; i < 8; ++i)
            {
                writer.WriteByte(0x47);
                writer.WriteByte(0x00);

                writer.WriteByte(data.stepResults[i].error.GetValue());
                writer.WriteByte(i);

                for (auto x : data.stepResults[i].RawMagnetometerMeasurement)
                {
                    writer.WriteSignedDoubleWordLE(x);
                }
                for (auto x : data.stepResults[i].CalibratedMagnetometerMeasurement)
                {
                    writer.WriteSignedDoubleWordLE(x);
                }
                for (auto x : data.stepResults[i].CoilCurrent)
                {
                    writer.WriteSignedWordLE(x);
                }
                for (auto x : data.stepResults[i].CoilTemperature)
                {
                    writer.WriteSignedWordLE(x);
                }
            }
            return I2CResult::OK;
        }));

        SelfTestResult result;
        RC_ASSERT(imtq.GetSelfTestResult(result));

        for (int i = 0; i < 8; ++i)
        {
            bool x = (result.stepResults[i] == data.stepResults[i]);
            RC_ASSERT(x);
        }
    }

    RC_GTEST_FIXTURE_PROP(ImtqTest, GetDetumbleData, ())
    {
        DetumbleData data;

        data.calibratedMagnetometerMeasurement = *rc::gen::arbitrary<Vector3<int32_t>>();
        data.filteredMagnetometerMeasurement = *rc::gen::arbitrary<Vector3<int32_t>>();
        data.bDotData = *rc::gen::arbitrary<Vector3<int32_t>>();
        data.commandedDipole = *rc::gen::arbitrary<Vector3<int16_t>>();
        data.commandedCurrent = *rc::gen::arbitrary<Vector3<int16_t>>();
        data.measuredCurrent = *rc::gen::arbitrary<Vector3<int16_t>>();

        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0x48))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Invoke([&](uint8_t /*address*/, auto outData) {
            EXPECT_TRUE(outData.size() == 56);

            Writer writer{outData};

            writer.WriteByte(0x48);
            writer.WriteByte(0x00);

            for (auto x : data.calibratedMagnetometerMeasurement)
            {
                writer.WriteSignedDoubleWordLE(x);
            }
            for (auto x : data.filteredMagnetometerMeasurement)
            {
                writer.WriteSignedDoubleWordLE(x);
            }
            for (auto x : data.bDotData)
            {
                writer.WriteSignedDoubleWordLE(x);
            }
            for (auto x : data.commandedDipole)
            {
                writer.WriteSignedWordLE(x);
            }
            for (auto x : data.commandedCurrent)
            {
                writer.WriteSignedWordLE(x);
            }
            for (auto x : data.measuredCurrent)
            {
                writer.WriteSignedWordLE(x);
            }
            return I2CResult::OK;
        }));

        DetumbleData result;
        RC_ASSERT(imtq.GetDetumbleData(result));

        RC_ASSERT(data.calibratedMagnetometerMeasurement == result.calibratedMagnetometerMeasurement);
    }

    RC_GTEST_FIXTURE_PROP(ImtqTest, GetHouseKeepingRAW, ())
    {
        HouseKeepingRAW data;

        data.digitalVoltage = *rc::gen::arbitrary<uint16_t>();
        data.analogVoltage = *rc::gen::arbitrary<uint16_t>();
        data.digitalCurrent = *rc::gen::arbitrary<uint16_t>();
        data.analogCurrent = *rc::gen::arbitrary<uint16_t>();

        data.coilCurrent = *rc::gen::arbitrary<Vector3<uint16_t>>();
        data.coilTemperature = *rc::gen::arbitrary<Vector3<uint16_t>>();

        data.MCUtemperature = *rc::gen::arbitrary<uint16_t>();

        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0x49))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Invoke([&](uint8_t /*address*/, auto outData) {
            EXPECT_TRUE(outData.size() == 24);

            Writer writer{outData};

            writer.WriteByte(0x49);
            writer.WriteByte(0x00);

            writer.WriteWordLE(data.digitalVoltage);
            writer.WriteWordLE(data.analogVoltage);
            writer.WriteWordLE(data.digitalCurrent);
            writer.WriteWordLE(data.analogCurrent);

            for (auto x : data.coilCurrent)
            {
                writer.WriteWordLE(x);
            }
            for (auto x : data.coilTemperature)
            {
                writer.WriteWordLE(x);
            }
            writer.WriteWordLE(data.MCUtemperature);
            return I2CResult::OK;
        }));

        HouseKeepingRAW result;
        RC_ASSERT(imtq.GetHouseKeepingRAW(result));

        RC_ASSERT(data.digitalVoltage == result.digitalVoltage);
        RC_ASSERT(data.analogVoltage == result.analogVoltage);
        RC_ASSERT(data.digitalCurrent == result.digitalCurrent);
        RC_ASSERT(data.analogCurrent == result.analogCurrent);
        RC_ASSERT(data.coilCurrent == result.coilCurrent);
        RC_ASSERT(data.coilTemperature == result.coilTemperature);
        RC_ASSERT(data.MCUtemperature == result.MCUtemperature);
    }

    RC_GTEST_FIXTURE_PROP(ImtqTest, GetHouseKeepingEngineering, ())
    {
        HouseKeepingEngineering data;

        data.digitalVoltage = *rc::gen::arbitrary<uint16_t>();
        data.analogVoltage = *rc::gen::arbitrary<uint16_t>();
        data.digitalCurrent = *rc::gen::arbitrary<uint16_t>();
        data.analogCurrent = *rc::gen::arbitrary<uint16_t>();

        data.coilCurrent = *rc::gen::arbitrary<Vector3<int16_t>>();
        data.coilTemperature = *rc::gen::arbitrary<Vector3<int16_t>>();

        data.MCUtemperature = *rc::gen::arbitrary<int16_t>();

        EXPECT_CALL(i2c, Write(ImtqAddress, ElementsAre(0x4A))).WillOnce(Return(I2CResult::OK));
        EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Invoke([&](uint8_t /*address*/, auto outData) {
            EXPECT_TRUE(outData.size() == 24);

            Writer writer{outData};

            writer.WriteByte(0x4A);
            writer.WriteByte(0x00);

            writer.WriteWordLE(data.digitalVoltage);
            writer.WriteWordLE(data.analogVoltage);
            writer.WriteWordLE(data.digitalCurrent);
            writer.WriteWordLE(data.analogCurrent);

            for (auto x : data.coilCurrent)
            {
                writer.WriteWordLE(x);
            }
            for (auto x : data.coilTemperature)
            {
                writer.WriteWordLE(x);
            }
            writer.WriteWordLE(data.MCUtemperature);
            return I2CResult::OK;
        }));

        HouseKeepingEngineering result;
        RC_ASSERT(imtq.GetHouseKeepingEngineering(result));

        RC_ASSERT(data.digitalVoltage == result.digitalVoltage);
        RC_ASSERT(data.analogVoltage == result.analogVoltage);
        RC_ASSERT(data.digitalCurrent == result.digitalCurrent);
        RC_ASSERT(data.analogCurrent == result.analogCurrent);
        RC_ASSERT(data.coilCurrent == result.coilCurrent);
        RC_ASSERT(data.coilTemperature == result.coilTemperature);
        RC_ASSERT(data.MCUtemperature == result.MCUtemperature);
    }

    RC_GTEST_FIXTURE_PROP(ImtqTest, SetParameter, ())
    {
        uint16_t param = *rc::gen::arbitrary<uint16_t>();
        const auto tab = *rc::gen::suchThat<std::vector<uint8_t>>([](std::vector<uint8_t> x) { return (x.size() > 0 && x.size() < 9); });

        EXPECT_CALL(i2c, Write(ImtqAddress, BeginsWith(0x82))).WillOnce(Invoke([&](uint8_t /*address*/, auto inData) {
            EXPECT_TRUE((size_t)inData.size() == 3u + tab.size());
            Reader reader{inData};
            reader.Skip(1);
            uint16_t id = reader.ReadWordLE();
            EXPECT_EQ(id, param);

            auto value = reader.ReadToEnd();
            EXPECT_EQ(value, gsl::make_span(tab));
            return I2CResult::OK;
        }));

        EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Invoke([&](uint8_t /*address*/, auto outData) {
            EXPECT_TRUE((size_t)outData.size() == 4u + tab.size());

            Writer writer{outData};

            writer.WriteByte(0x82);
            writer.WriteByte(0x00);
            writer.WriteWordLE(param);
            writer.WriteArray(tab);
            return I2CResult::OK;
        }));

        RC_ASSERT(imtq.SetParameter(param, tab));
    }

    RC_GTEST_FIXTURE_PROP(ImtqTest, GetParameter, ())
    {
        uint16_t param = *rc::gen::arbitrary<uint16_t>();
        const auto tab = *rc::gen::suchThat<std::vector<uint8_t>>([](std::vector<uint8_t> x) { return (x.size() > 0 && x.size() < 9); });

        EXPECT_CALL(i2c, Write(ImtqAddress, BeginsWith(0x81))).WillOnce(Invoke([&](uint8_t /*address*/, auto inData) {
            EXPECT_TRUE(inData.size() == 3);

            Reader reader{inData};
            reader.Skip(1);
            uint16_t id = reader.ReadWordLE();
            EXPECT_EQ(id, param);

            return I2CResult::OK;
        }));

        EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Invoke([&](uint8_t /*address*/, auto outData) {
            EXPECT_TRUE((size_t)outData.size() == 4u + tab.size());

            Writer writer{outData};

            writer.WriteByte(0x81);
            writer.WriteByte(0x00);
            writer.WriteWordLE(param);
            writer.WriteArray(tab);
            return I2CResult::OK;
        }));

        std::vector<uint8_t> result;
        result.resize(tab.size());

        RC_ASSERT(imtq.GetParameter(param, result));

        RC_ASSERT(tab == result);
    }

    RC_GTEST_FIXTURE_PROP(ImtqTest, ResetParameter, ())
    {
        uint16_t param = *rc::gen::arbitrary<uint16_t>();
        const auto tab = *rc::gen::suchThat<std::vector<uint8_t>>([](std::vector<uint8_t> x) { return (x.size() > 0 && x.size() < 9); });

        EXPECT_CALL(i2c, Write(ImtqAddress, BeginsWith(0x83))).WillOnce(Invoke([&](uint8_t /*address*/, auto inData) {
            EXPECT_TRUE(inData.size() == 3);

            Reader reader{inData};
            reader.Skip(1);
            uint16_t id = reader.ReadWordLE();
            EXPECT_EQ(id, param);

            return I2CResult::OK;
        }));

        EXPECT_CALL(i2c, Read(ImtqAddress, _)).WillOnce(Invoke([&](uint8_t /*address*/, auto outData) {
            EXPECT_TRUE((size_t)outData.size() == 4u + tab.size());

            Writer writer{outData};

            writer.WriteByte(0x83);
            writer.WriteByte(0x00);

            writer.WriteWordLE(param);
            writer.WriteArray(tab);
            return I2CResult::OK;
        }));

        std::vector<uint8_t> result;
        result.resize(tab.size());

        RC_ASSERT(imtq.ResetParameterAndGetDefault(param, result));

        RC_ASSERT(tab == result);
    }
}
