#include "gtest/gtest.h"
#include "mock/PayloadHardwareDriverMock.hpp"

#include "payload/devices.h"

using namespace drivers::payload;

using testing::Return;
using testing::_;
using testing::Eq;
using testing::Ge;
using testing::Ne;
using testing::Invoke;
using testing::ElementsAre;
using gsl::span;

namespace
{
    class PayloadDeviceDriverTest : public testing::Test
    {
      protected:
        PayloadDeviceDriverTest();

        PayloadHardwareDriverMock driver;
        PayloadDeviceDriver payload;
    };

    PayloadDeviceDriverTest::PayloadDeviceDriverTest() : payload(driver)
    {
    }

    TEST_F(PayloadDeviceDriverTest, GetWhoamiSuccessful)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return false; }));

        EXPECT_CALL(driver, PayloadRead(ElementsAre(0x00), _))
            .Times(1)
            .WillOnce(Invoke([=](span<const uint8_t> /*inData*/, span<uint8_t> outData) {
                std::fill(outData.begin(), outData.end(), 0x53);
                return OSResult::Success;
            }));

        PayloadTelemetry::Status result;

        ASSERT_THAT(payload.GetWhoami(result), Eq(OSResult::Success));
        ASSERT_THAT(result.who_am_i, Eq(0x53u));
    }

    TEST_F(PayloadDeviceDriverTest, GetWhoamiFailed)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return false; }));

        EXPECT_CALL(driver, PayloadRead(ElementsAre(0x00), _))
            .Times(1)
            .WillOnce(Invoke([=](span<const uint8_t> /*inData*/, span<uint8_t> outData) {
                std::fill(outData.begin(), outData.end(), 0xFF);
                return OSResult::InvalidOperation;
            }));

        PayloadTelemetry::Status result;
        ASSERT_THAT(payload.GetWhoami(result), Eq(OSResult::InvalidOperation));
    }

    TEST_F(PayloadDeviceDriverTest, GetWhoamiBusy)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return true; }));

        EXPECT_CALL(driver, PayloadRead(_, _)).Times(0);
        EXPECT_CALL(driver, PayloadWrite(_)).Times(0);
        EXPECT_CALL(driver, WaitForData()).Times(0);

        PayloadTelemetry::Status result;
        ASSERT_THAT(payload.GetWhoami(result), Eq(OSResult::Busy));
    }

    TEST_F(PayloadDeviceDriverTest, MeasureHousekeepingBusy)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return true; }));

        EXPECT_CALL(driver, PayloadRead(_, _)).Times(0);
        EXPECT_CALL(driver, PayloadWrite(_)).Times(0);
        EXPECT_CALL(driver, WaitForData()).Times(0);

        PayloadTelemetry::Housekeeping result;
        ASSERT_THAT(payload.MeasureHousekeeping(result), Eq(OSResult::Busy));
    }

    TEST_F(PayloadDeviceDriverTest, MeasurePhotodiodesBusy)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return true; }));

        EXPECT_CALL(driver, PayloadRead(_, _)).Times(0);
        EXPECT_CALL(driver, PayloadWrite(_)).Times(0);
        EXPECT_CALL(driver, WaitForData()).Times(0);

        PayloadTelemetry::Photodiodes result;
        ASSERT_THAT(payload.MeasurePhotodiodes(result), Eq(OSResult::Busy));
    }

    TEST_F(PayloadDeviceDriverTest, MeasureTemperaturesBusy)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return true; }));

        EXPECT_CALL(driver, PayloadRead(_, _)).Times(0);
        EXPECT_CALL(driver, PayloadWrite(_)).Times(0);
        EXPECT_CALL(driver, WaitForData()).Times(0);

        PayloadTelemetry::Temperatures result;
        ASSERT_THAT(payload.MeasureTemperatures(result), Eq(OSResult::Busy));
    }

    TEST_F(PayloadDeviceDriverTest, MeasureSunSRefBusy)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return true; }));

        EXPECT_CALL(driver, PayloadRead(_, _)).Times(0);
        EXPECT_CALL(driver, PayloadWrite(_)).Times(0);
        EXPECT_CALL(driver, WaitForData()).Times(0);

        PayloadTelemetry::SunsRef result;
        ASSERT_THAT(payload.MeasureSunSRef(result), Eq(OSResult::Busy));
    }

    TEST_F(PayloadDeviceDriverTest, MeasureRadFETOnBusy)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return true; }));

        EXPECT_CALL(driver, PayloadRead(_, _)).Times(0);
        EXPECT_CALL(driver, PayloadWrite(_)).Times(0);
        EXPECT_CALL(driver, WaitForData()).Times(0);

        PayloadTelemetry::Radfet result;
        ASSERT_THAT(payload.RadFETOn(result), Eq(OSResult::Busy));
    }

    TEST_F(PayloadDeviceDriverTest, MeasureRadFETOffBusy)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return true; }));

        EXPECT_CALL(driver, PayloadRead(_, _)).Times(0);
        EXPECT_CALL(driver, PayloadWrite(_)).Times(0);
        EXPECT_CALL(driver, WaitForData()).Times(0);

        PayloadTelemetry::Radfet result;
        ASSERT_THAT(payload.RadFETOff(result), Eq(OSResult::Busy));
    }

    TEST_F(PayloadDeviceDriverTest, MeasureRadFETReadBusy)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return true; }));

        EXPECT_CALL(driver, PayloadRead(_, _)).Times(0);
        EXPECT_CALL(driver, PayloadWrite(_)).Times(0);
        EXPECT_CALL(driver, WaitForData()).Times(0);

        PayloadTelemetry::Radfet result;
        ASSERT_THAT(payload.MeasureRadFET(result), Eq(OSResult::Busy));
    }

    TEST_F(PayloadDeviceDriverTest, MeasureRadFETOnSuccessful)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return false; }));

        EXPECT_CALL(driver, PayloadWrite(ElementsAre(0x84))).Times(1).WillOnce(Invoke([=](span<const uint8_t> /*outData*/) {
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, PayloadRead(ElementsAre(41), _)).WillOnce(Invoke([=](span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0x01);
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, WaitForData()).Times(1);

        PayloadTelemetry::Radfet result;
        ASSERT_THAT(payload.RadFETOn(result), Eq(OSResult::Success));

        ASSERT_THAT(result.status, Eq(0x01u));
        ASSERT_THAT(result.temperature, Eq(0x01010101u));
        ASSERT_THAT(result.vth[0], Eq(0x01010101u));
        ASSERT_THAT(result.vth[1], Eq(0x01010101u));
        ASSERT_THAT(result.vth[2], Eq(0x01010101u));
    }

    TEST_F(PayloadDeviceDriverTest, MeasureRadFETSuccessful)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return false; }));

        EXPECT_CALL(driver, PayloadWrite(ElementsAre(0x85))).Times(1).WillOnce(Invoke([=](span<const uint8_t> /*outData*/) {
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, PayloadRead(ElementsAre(41), _)).WillOnce(Invoke([=](span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0x01);
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, WaitForData()).Times(1);

        PayloadTelemetry::Radfet result;
        ASSERT_THAT(payload.MeasureRadFET(result), Eq(OSResult::Success));

        ASSERT_THAT(result.status, Eq(0x01u));
        ASSERT_THAT(result.temperature, Eq(0x01010101u));
        ASSERT_THAT(result.vth[0], Eq(0x01010101u));
        ASSERT_THAT(result.vth[1], Eq(0x01010101u));
        ASSERT_THAT(result.vth[2], Eq(0x01010101u));
    }

    TEST_F(PayloadDeviceDriverTest, MeasureRadFETOffSuccessful)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return false; }));

        EXPECT_CALL(driver, PayloadWrite(ElementsAre(0x86))).Times(1).WillOnce(Invoke([=](span<const uint8_t> /*outData*/) {
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, PayloadRead(ElementsAre(41), _)).WillOnce(Invoke([=](span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0x01);
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, WaitForData()).Times(1);

        PayloadTelemetry::Radfet result;
        ASSERT_THAT(payload.RadFETOff(result), Eq(OSResult::Success));

        ASSERT_THAT(result.status, Eq(0x01u));
        ASSERT_THAT(result.temperature, Eq(0x01010101u));
        ASSERT_THAT(result.vth[0], Eq(0x01010101u));
        ASSERT_THAT(result.vth[1], Eq(0x01010101u));
        ASSERT_THAT(result.vth[2], Eq(0x01010101u));
    }

    TEST_F(PayloadDeviceDriverTest, MeasureRadFETTimeout)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return false; }));

        EXPECT_CALL(driver, PayloadWrite(ElementsAre(0x85))).Times(1).WillOnce(Invoke([=](span<const uint8_t> /*outData*/) {
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, WaitForData()).Times(1).WillOnce(Invoke([=]() { return OSResult::Timeout; }));

        EXPECT_CALL(driver, PayloadRead(ElementsAre(41), _)).Times(0);

        PayloadTelemetry::Radfet result;
        ASSERT_THAT(payload.MeasureRadFET(result), Eq(OSResult::Timeout));
    }

    TEST_F(PayloadDeviceDriverTest, MeasureRadFETFailed)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return false; }));

        EXPECT_CALL(driver, PayloadWrite(ElementsAre(0x85))).Times(1).WillOnce(Invoke([=](span<const uint8_t> /*outData*/) {
            return OSResult::InvalidOperation;
        }));

        EXPECT_CALL(driver, WaitForData()).Times(0);
        EXPECT_CALL(driver, PayloadRead(_, _)).Times(0);

        PayloadTelemetry::Radfet result;
        ASSERT_THAT(payload.MeasureRadFET(result), Eq(OSResult::InvalidOperation));
    }

    TEST_F(PayloadDeviceDriverTest, MeasureRadFETFailedRead)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return false; }));

        EXPECT_CALL(driver, PayloadWrite(ElementsAre(0x85))).Times(1).WillOnce(Invoke([=](span<const uint8_t> /*outData*/) {
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, WaitForData()).Times(1);

        EXPECT_CALL(driver, PayloadRead(ElementsAre(41), _)).WillOnce(Invoke([=](span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0x01);
            return OSResult::InvalidAddress;
        }));

        PayloadTelemetry::Radfet result;
        ASSERT_THAT(payload.MeasureRadFET(result), Eq(OSResult::InvalidAddress));
    }

    TEST_F(PayloadDeviceDriverTest, MeasureSunSFailed)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return false; }));

        EXPECT_CALL(driver, PayloadWrite(ElementsAre(0x80))).Times(1).WillOnce(Invoke([=](span<const uint8_t> /*outData*/) {
            return OSResult::InvalidOperation;
        }));

        EXPECT_CALL(driver, WaitForData()).Times(0);
        EXPECT_CALL(driver, PayloadRead(_, _)).Times(0);

        PayloadTelemetry::SunsRef result;
        ASSERT_THAT(payload.MeasureSunSRef(result), Eq(OSResult::InvalidOperation));
    }

    TEST_F(PayloadDeviceDriverTest, MeasureTemperaturesFailed)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return false; }));

        EXPECT_CALL(driver, PayloadWrite(ElementsAre(0x81))).Times(1).WillOnce(Invoke([=](span<const uint8_t> /*outData*/) {
            return OSResult::InvalidOperation;
        }));

        EXPECT_CALL(driver, WaitForData()).Times(0);
        EXPECT_CALL(driver, PayloadRead(_, _)).Times(0);

        PayloadTelemetry::Temperatures result;
        ASSERT_THAT(payload.MeasureTemperatures(result), Eq(OSResult::InvalidOperation));
    }

    TEST_F(PayloadDeviceDriverTest, MeasureSunPhotodiodesFailed)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return false; }));

        EXPECT_CALL(driver, PayloadWrite(ElementsAre(0x82))).Times(1).WillOnce(Invoke([=](span<const uint8_t> /*outData*/) {
            return OSResult::InvalidOperation;
        }));

        EXPECT_CALL(driver, WaitForData()).Times(0);
        EXPECT_CALL(driver, PayloadRead(_, _)).Times(0);

        PayloadTelemetry::Photodiodes result;
        ASSERT_THAT(payload.MeasurePhotodiodes(result), Eq(OSResult::InvalidOperation));
    }

    TEST_F(PayloadDeviceDriverTest, MeasureHousekeepingFailed)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return false; }));

        EXPECT_CALL(driver, PayloadWrite(ElementsAre(0x83))).Times(1).WillOnce(Invoke([=](span<const uint8_t> /*outData*/) {
            return OSResult::InvalidOperation;
        }));

        EXPECT_CALL(driver, WaitForData()).Times(0);
        EXPECT_CALL(driver, PayloadRead(_, _)).Times(0);

        PayloadTelemetry::Housekeeping result;
        ASSERT_THAT(payload.MeasureHousekeeping(result), Eq(OSResult::InvalidOperation));
    }

    TEST_F(PayloadDeviceDriverTest, MeasureSunSSuccessful)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return false; }));

        EXPECT_CALL(driver, PayloadWrite(ElementsAre(0x80))).Times(1).WillOnce(Invoke([=](span<const uint8_t> /*outData*/) {
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, PayloadRead(ElementsAre(1), _)).WillOnce(Invoke([=](span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0x02);
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, WaitForData()).Times(1);

        PayloadTelemetry::SunsRef result;
        ASSERT_THAT(payload.MeasureSunSRef(result), Eq(OSResult::Success));

        ASSERT_THAT(result.voltages[0], Eq(0x0202u));
        ASSERT_THAT(result.voltages[1], Eq(0x0202u));
        ASSERT_THAT(result.voltages[2], Eq(0x0202u));
        ASSERT_THAT(result.voltages[3], Eq(0x0202u));
        ASSERT_THAT(result.voltages[4], Eq(0x0202u));
    }

    TEST_F(PayloadDeviceDriverTest, MeasureTemperaturesSuccessful)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return false; }));

        EXPECT_CALL(driver, PayloadWrite(ElementsAre(0x81))).Times(1).WillOnce(Invoke([=](span<const uint8_t> /*outData*/) {
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, PayloadRead(ElementsAre(11), _)).WillOnce(Invoke([=](span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0x23);
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, WaitForData()).Times(1);

        PayloadTelemetry::Temperatures result;
        ASSERT_THAT(payload.MeasureTemperatures(result), Eq(OSResult::Success));

        ASSERT_THAT(result.supply, Eq(0x2323u));
        ASSERT_THAT(result.Xp, Eq(0x2323u));
        ASSERT_THAT(result.Xn, Eq(0x2323u));
        ASSERT_THAT(result.Yp, Eq(0x2323u));
        ASSERT_THAT(result.Yn, Eq(0x2323u));
        ASSERT_THAT(result.sads, Eq(0x2323u));
        ASSERT_THAT(result.sail, Eq(0x2323u));
        ASSERT_THAT(result.cam_nadir, Eq(0x2323u));
        ASSERT_THAT(result.cam_wing, Eq(0x2323u));
    }

    TEST_F(PayloadDeviceDriverTest, MeasurePhotodiodesSuccessful)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return false; }));

        EXPECT_CALL(driver, PayloadWrite(ElementsAre(0x82))).Times(1).WillOnce(Invoke([=](span<const uint8_t> /*outData*/) {
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, PayloadRead(ElementsAre(29), _)).WillOnce(Invoke([=](span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0x24);
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, WaitForData()).Times(1);

        PayloadTelemetry::Photodiodes result;
        ASSERT_THAT(payload.MeasurePhotodiodes(result), Eq(OSResult::Success));

        ASSERT_THAT(result.Xp, Eq(0x2424u));
        ASSERT_THAT(result.Xn, Eq(0x2424u));
        ASSERT_THAT(result.Yp, Eq(0x2424u));
        ASSERT_THAT(result.Yn, Eq(0x2424u));
    }

    TEST_F(PayloadDeviceDriverTest, MeasureHousekeepingSuccessful)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return false; }));

        EXPECT_CALL(driver, PayloadWrite(ElementsAre(0x83))).Times(1).WillOnce(Invoke([=](span<const uint8_t> /*outData*/) {
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, PayloadRead(ElementsAre(37), _)).WillOnce(Invoke([=](span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0x25);
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, WaitForData()).Times(1);

        PayloadTelemetry::Housekeeping result;
        ASSERT_THAT(payload.MeasureHousekeeping(result), Eq(OSResult::Success));

        ASSERT_THAT(result.int_3v3d, Eq(0x2525u));
        ASSERT_THAT(result.obc_3v3d, Eq(0x2525u));
    }

    TEST_F(PayloadDeviceDriverTest, SunSValidationDoesntCancelProcessing)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return false; }));

        EXPECT_CALL(driver, PayloadWrite(ElementsAre(0x80))).Times(1).WillOnce(Invoke([=](span<const uint8_t> /*outData*/) {
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, PayloadRead(ElementsAre(1), _)).WillOnce(Invoke([=](span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0xFFu);
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, WaitForData()).Times(1);

        PayloadTelemetry::SunsRef result;
        ASSERT_THAT(payload.MeasureSunSRef(result), Eq(OSResult::Success));
    }

    TEST_F(PayloadDeviceDriverTest, TemperaturesValidationDoesntCancelProcessing)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return false; }));

        EXPECT_CALL(driver, PayloadWrite(ElementsAre(0x81))).Times(1).WillOnce(Invoke([=](span<const uint8_t> /*outData*/) {
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, PayloadRead(ElementsAre(11), _)).WillOnce(Invoke([=](span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0xFFu);
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, WaitForData()).Times(1);

        PayloadTelemetry::Temperatures result;
        ASSERT_THAT(payload.MeasureTemperatures(result), Eq(OSResult::Success));
    }

    TEST_F(PayloadDeviceDriverTest, PhotodiodesValidationDoesntCancelProcessing)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return false; }));

        EXPECT_CALL(driver, PayloadWrite(ElementsAre(0x82))).Times(1).WillOnce(Invoke([=](span<const uint8_t> /*outData*/) {
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, PayloadRead(ElementsAre(29), _)).WillOnce(Invoke([=](span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0xFFu);
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, WaitForData()).Times(1);

        PayloadTelemetry::Photodiodes result;
        ASSERT_THAT(payload.MeasurePhotodiodes(result), Eq(OSResult::Success));
    }

    TEST_F(PayloadDeviceDriverTest, HousekeepingValidationDoesntCancelProcessing)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return false; }));

        EXPECT_CALL(driver, PayloadWrite(ElementsAre(0x83))).Times(1).WillOnce(Invoke([=](span<const uint8_t> /*outData*/) {
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, PayloadRead(ElementsAre(37), _)).WillOnce(Invoke([=](span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0xFFu);
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, WaitForData()).Times(1);

        PayloadTelemetry::Housekeeping result;
        ASSERT_THAT(payload.MeasureHousekeeping(result), Eq(OSResult::Success));
    }

    TEST_F(PayloadDeviceDriverTest, RadFETValidationDoesntCancelProcessing)
    {
        EXPECT_CALL(driver, IsBusy()).WillOnce(Invoke([]() { return false; }));

        EXPECT_CALL(driver, PayloadWrite(ElementsAre(0x86))).Times(1).WillOnce(Invoke([=](span<const uint8_t> /*outData*/) {
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, PayloadRead(ElementsAre(41), _)).WillOnce(Invoke([=](span<const uint8_t> /*inData*/, span<uint8_t> outData) {
            std::fill(outData.begin(), outData.end(), 0xFFu);
            return OSResult::Success;
        }));

        EXPECT_CALL(driver, WaitForData()).Times(1);

        PayloadTelemetry::Radfet result;
        ASSERT_THAT(payload.RadFETOff(result), Eq(OSResult::Success));
    }
}
