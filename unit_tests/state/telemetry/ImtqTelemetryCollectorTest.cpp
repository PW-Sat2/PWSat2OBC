#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "OsMock.hpp"
#include "mock/ImtqDriverMock.hpp"
#include "os/os.hpp"
#include "telemetry/ImtqTelemetryCollector.hpp"
#include "telemetry/state.hpp"

namespace
{
    using testing::_;
    using testing::Eq;
    using testing::Invoke;
    using testing::Return;
    using namespace devices::imtq;
    using namespace std::chrono_literals;
    using namespace telemetry;
    typedef devices::imtq::ImtqTelemetryCollector::ElementId ElementId;

    class ImtqTelemetryCollectorTest : public ::testing::Test
    {
      protected:
        ImtqTelemetryCollectorTest();

        void MockTelemetryLock();
        void MockTelemetryLockFailure();
        void MockNoLocking();

        void TestCaptureFailure(ElementId id);
        void CaptureTelemetry();
        void TestIsNew(ElementId id);
        void TestIsOld(ElementId id);

        template <typename T> void Capture(T& element, ElementId id);

        ImtqDriverMock driver;
        ImtqTelemetryCollector collector;
        telemetry::ManagedTelemetry state;
        testing::NiceMock<OSMock> os;
        OSReset osReset;
    };

    ImtqTelemetryCollectorTest::ImtqTelemetryCollectorTest() : collector(driver)
    {
        osReset = InstallProxy(&os);
    }

    void ImtqTelemetryCollectorTest::MockTelemetryLock()
    {
        EXPECT_CALL(os, TakeSemaphore(_, 50ms)).WillOnce(Invoke([&](auto handle, auto /*timeout*/) {
            EXPECT_CALL(os, GiveSemaphore(handle)).WillOnce(Return(OSResult::Success));
            return OSResult::Success;
        }));
    }

    void ImtqTelemetryCollectorTest::MockTelemetryLockFailure()
    {
        EXPECT_CALL(os, TakeSemaphore(_, 50ms)).WillOnce(Return(OSResult::Timeout));
        EXPECT_CALL(os, GiveSemaphore(_)).Times(0);
    }

    void ImtqTelemetryCollectorTest::MockNoLocking()
    {
        EXPECT_CALL(os, TakeSemaphore(_, _)).Times(0);
        EXPECT_CALL(os, GiveSemaphore(_)).Times(0);
    }

    void ImtqTelemetryCollectorTest::CaptureTelemetry()
    {
        MockTelemetryLock();
        const auto status = collector.CaptureTelemetry(state);
        ASSERT_THAT(status, Eq(true));
    }

    template <typename T> void ImtqTelemetryCollectorTest::Capture(T& element, ElementId id)
    {
        TestIsNew(id);
        CaptureTelemetry();
        TestIsOld(id);
        element = state.Get<T>();
    }

    void ImtqTelemetryCollectorTest::TestCaptureFailure(ElementId id)
    {
        TestIsNew(id);
        MockTelemetryLockFailure();
        const auto status = collector.CaptureTelemetry(state);
        ASSERT_THAT(status, Eq(false));
        TestIsNew(id);
    }

    void ImtqTelemetryCollectorTest::TestIsNew(ElementId id)
    {
        ASSERT_THAT(collector.IsNew(id), Eq(true));
    }

    void ImtqTelemetryCollectorTest::TestIsOld(ElementId id)
    {
        ASSERT_THAT(collector.IsNew(id), Eq(false));
    }

    TEST_F(ImtqTelemetryCollectorTest, TestSelfTestFailure)
    {
        SelfTestResult result;
        EXPECT_CALL(driver, PerformSelfTest(_, _)).WillOnce(Return(false));
        const auto status = collector.PerformSelfTest(result, true);
        ASSERT_THAT(status, Eq(false));
        TestIsOld(ElementId::SelfTest);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestSelfTestSuccess)
    {
        MockTelemetryLock();
        SelfTestResult result;
        EXPECT_CALL(driver, PerformSelfTest(_, _)).WillOnce(Return(true));
        const auto status = collector.PerformSelfTest(result, true);
        ASSERT_THAT(status, Eq(true));
        TestIsNew(ElementId::SelfTest);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestSelfTestTelemetryLockingFailure)
    {
        SelfTestResult result;
        MockTelemetryLockFailure();
        EXPECT_CALL(driver, PerformSelfTest(_, _)).WillOnce(Return(true));
        const auto status = collector.PerformSelfTest(result, true);
        ASSERT_THAT(status, Eq(true));
        TestIsOld(ElementId::SelfTest);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestSelfTestTelemetryCaptureFailure)
    {
        SelfTestResult result;
        MockTelemetryLock();
        EXPECT_CALL(driver, PerformSelfTest(_, _)).WillOnce(Return(true));
        collector.PerformSelfTest(result, true);
        TestCaptureFailure(ElementId::SelfTest);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestSelfTestTelemetry)
    {
        SelfTestResult result;
        EXPECT_CALL(driver, PerformSelfTest(_, _)).WillOnce(Invoke([](SelfTestResult& result, bool /*fix*/) {
            int i = 0;
            for (auto& entry : result.stepResults)
            {
                entry.error = i++;
            }

            return true;
        }));

        collector.PerformSelfTest(result, true);
        ImtqSelfTest selfTest;
        Capture(selfTest, ElementId::SelfTest);
        int i = 0;
        for (auto entry : selfTest.GetValue())
        {
            EXPECT_THAT(entry, Eq(i));
            ++i;
        }
    }

    TEST_F(ImtqTelemetryCollectorTest, TestSelfTestResultSuccess)
    {
        MockTelemetryLock();
        SelfTestResult result;
        EXPECT_CALL(driver, GetSelfTestResult(_)).WillOnce(Return(true));
        const auto status = collector.GetSelfTestResult(result);
        ASSERT_THAT(status, Eq(true));
        TestIsNew(ElementId::SelfTest);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestSelfTestResulTelemetryLockingFailure)
    {
        SelfTestResult result;
        MockTelemetryLockFailure();
        EXPECT_CALL(driver, GetSelfTestResult(_)).WillOnce(Return(true));
        const auto status = collector.GetSelfTestResult(result);
        ASSERT_THAT(status, Eq(true));
        TestIsOld(ElementId::SelfTest);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestSelfTestResulTelemetryCaptureFailure)
    {
        SelfTestResult result;
        MockTelemetryLock();
        EXPECT_CALL(driver, GetSelfTestResult(_)).WillOnce(Return(true));
        collector.GetSelfTestResult(result);
        TestCaptureFailure(ElementId::SelfTest);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestSelfTestResulTelemetry)
    {
        SelfTestResult result;
        EXPECT_CALL(driver, GetSelfTestResult(_)).WillOnce(Invoke([](SelfTestResult& result) {
            int i = 0;
            for (auto& entry : result.stepResults)
            {
                entry.error = i++;
            }

            return true;
        }));

        collector.GetSelfTestResult(result);
        ImtqSelfTest selfTest;
        Capture(selfTest, ElementId::SelfTest);
        int i = 0;
        for (auto entry : selfTest.GetValue())
        {
            EXPECT_THAT(entry, Eq(i));
            ++i;
        }
    }

    TEST_F(ImtqTelemetryCollectorTest, TestMeasureMagnetometerFailure)
    {
        Vector3<MagnetometerMeasurement> result;
        EXPECT_CALL(driver, MeasureMagnetometer(_)).WillOnce(Return(false));
        const auto status = collector.MeasureMagnetometer(result);
        ASSERT_THAT(status, Eq(false));
        TestIsOld(ElementId::Magnetometer);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestMeasureMagnetometerSuccess)
    {
        MockTelemetryLock();
        Vector3<MagnetometerMeasurement> result;
        EXPECT_CALL(driver, MeasureMagnetometer(_)).WillOnce(Return(true));
        const auto status = collector.MeasureMagnetometer(result);
        ASSERT_THAT(status, Eq(true));
        TestIsNew(ElementId::Magnetometer);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestMeasureMagnetometerLockFailure)
    {
        MockTelemetryLockFailure();
        Vector3<MagnetometerMeasurement> result;
        EXPECT_CALL(driver, MeasureMagnetometer(_)).WillOnce(Return(true));
        const auto status = collector.MeasureMagnetometer(result);
        ASSERT_THAT(status, Eq(true));
        TestIsOld(ElementId::Magnetometer);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestMeasureMagnetometerTelemetry)
    {
        MockTelemetryLock();
        Vector3<MagnetometerMeasurement> result;
        EXPECT_CALL(driver, MeasureMagnetometer(_)).WillOnce(Invoke([](auto& result) {
            int i = 0;
            for (auto& entry : result)
            {
                entry = i++;
            }

            return true;
        }));

        collector.MeasureMagnetometer(result);

        ImtqMagnetometerMeasurements telemetry;
        Capture(telemetry, ElementId::Magnetometer);
        int i = 0;
        for (auto entry : telemetry.GetValue())
        {
            EXPECT_THAT(entry, Eq(i));
            ++i;
        }
    }

    TEST_F(ImtqTelemetryCollectorTest, TestSoftwareReset)
    {
        MockNoLocking();
        EXPECT_CALL(driver, SoftwareReset()).WillOnce(Return(true)).WillOnce(Return(false));
        ASSERT_THAT(collector.SoftwareReset(), Eq(true));
        ASSERT_THAT(collector.SoftwareReset(), Eq(false));
    }

    TEST_F(ImtqTelemetryCollectorTest, TestSendNoOperation)
    {
        MockNoLocking();
        EXPECT_CALL(driver, SendNoOperation()).WillOnce(Return(true)).WillOnce(Return(false));
        ASSERT_THAT(collector.SendNoOperation(), Eq(true));
        ASSERT_THAT(collector.SendNoOperation(), Eq(false));
    }

    TEST_F(ImtqTelemetryCollectorTest, TestCancelOperation)
    {
        MockNoLocking();
        EXPECT_CALL(driver, CancelOperation()).WillOnce(Return(true)).WillOnce(Return(false));
        ASSERT_THAT(collector.CancelOperation(), Eq(true));
        ASSERT_THAT(collector.CancelOperation(), Eq(false));
    }

    TEST_F(ImtqTelemetryCollectorTest, TestStartMTMMeasurement)
    {
        MockNoLocking();
        EXPECT_CALL(driver, StartMTMMeasurement()).WillOnce(Return(true)).WillOnce(Return(false));
        ASSERT_THAT(collector.StartMTMMeasurement(), Eq(true));
        ASSERT_THAT(collector.StartMTMMeasurement(), Eq(false));
    }

    TEST_F(ImtqTelemetryCollectorTest, TestStartActuationCurrent)
    {
        MockNoLocking();
        EXPECT_CALL(driver, StartActuationCurrent(Vector3<Current>{}, 100ms)).WillOnce(Return(true)).WillOnce(Return(false));
        ASSERT_THAT(collector.StartActuationCurrent(Vector3<Current>{}, 100ms), Eq(true));
        ASSERT_THAT(collector.StartActuationCurrent(Vector3<Current>{}, 100ms), Eq(false));
    }

    TEST_F(ImtqTelemetryCollectorTest, TestStartAllAxisSelfTest)
    {
        MockNoLocking();
        EXPECT_CALL(driver, StartAllAxisSelfTest()).WillOnce(Return(true)).WillOnce(Return(false));
        ASSERT_THAT(collector.StartAllAxisSelfTest(), Eq(true));
        ASSERT_THAT(collector.StartAllAxisSelfTest(), Eq(false));
    }

    TEST_F(ImtqTelemetryCollectorTest, TestStartBDotDetumbling)
    {
        MockNoLocking();
        EXPECT_CALL(driver, StartBDotDetumbling(2s)).WillOnce(Return(true)).WillOnce(Return(false));
        ASSERT_THAT(collector.StartBDotDetumbling(2s), Eq(true));
        ASSERT_THAT(collector.StartBDotDetumbling(2s), Eq(false));
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetHouseKeepingRAW)
    {
        HouseKeepingRAW result;
        MockNoLocking();
        EXPECT_CALL(driver, GetHouseKeepingRAW(_)).WillOnce(Return(true)).WillOnce(Return(false));
        ASSERT_THAT(collector.GetHouseKeepingRAW(result), Eq(true));
        ASSERT_THAT(collector.GetHouseKeepingRAW(result), Eq(false));
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetParameter)
    {
        MockNoLocking();
        EXPECT_CALL(driver, GetParameter(0x7fde, _)).WillOnce(Return(true)).WillOnce(Return(false));
        ASSERT_THAT(collector.GetParameter(0x7fde, gsl::span<std::uint8_t>()), Eq(true));
        ASSERT_THAT(collector.GetParameter(0x7fde, gsl::span<std::uint8_t>()), Eq(false));
    }

    TEST_F(ImtqTelemetryCollectorTest, TestSetParameter)
    {
        MockNoLocking();
        EXPECT_CALL(driver, SetParameter(0x7fde, _)).WillOnce(Return(true)).WillOnce(Return(false));
        ASSERT_THAT(collector.SetParameter(0x7fde, gsl::span<const std::uint8_t>()), Eq(true));
        ASSERT_THAT(collector.SetParameter(0x7fde, gsl::span<const std::uint8_t>()), Eq(false));
    }

    TEST_F(ImtqTelemetryCollectorTest, TestResetParameterAndGetDefault)
    {
        MockNoLocking();
        EXPECT_CALL(driver, ResetParameterAndGetDefault(0x7fde, _)).WillOnce(Return(true)).WillOnce(Return(false));
        ASSERT_THAT(collector.ResetParameterAndGetDefault(0x7fde, gsl::span<std::uint8_t>()), Eq(true));
        ASSERT_THAT(collector.ResetParameterAndGetDefault(0x7fde, gsl::span<std::uint8_t>()), Eq(false));
    }

    TEST_F(ImtqTelemetryCollectorTest, TestStartActuationDipoleFailure)
    {
        EXPECT_CALL(driver, StartActuationDipole(_, _)).WillOnce(Return(false));
        const auto status = collector.StartActuationDipole(Vector3<Dipole>{}, 200ms);
        ASSERT_THAT(status, Eq(false));
        TestIsOld(ElementId::Dipoles);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestStartActuationDipoleSuccess)
    {
        MockTelemetryLock();
        EXPECT_CALL(driver, StartActuationDipole(_, _)).WillOnce(Return(true));
        const auto status = collector.StartActuationDipole(Vector3<Dipole>{}, 200ms);
        ASSERT_THAT(status, Eq(true));
        TestIsNew(ElementId::Dipoles);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestStartActuationDipoleLockFailure)
    {
        MockTelemetryLockFailure();
        EXPECT_CALL(driver, StartActuationDipole(_, _)).WillOnce(Return(true));
        const auto status = collector.StartActuationDipole(Vector3<Dipole>{}, 200ms);
        ASSERT_THAT(status, Eq(true));
        TestIsOld(ElementId::Dipoles);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestStartActuationDipoleTelemetry)
    {
        MockTelemetryLock();
        EXPECT_CALL(driver, StartActuationDipole(Vector3<Dipole>{1, 2, 3}, 200ms)).WillOnce(Return(true));

        collector.StartActuationDipole(Vector3<Dipole>{1, 2, 3}, 200ms);

        ImtqDipoles telemetry;
        Capture(telemetry, ElementId::Dipoles);
        int i = 1;
        for (auto entry : telemetry.GetValue())
        {
            EXPECT_THAT(entry, Eq(i));
            ++i;
        }
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetSystemStateFailure)
    {
        State state;
        EXPECT_CALL(driver, GetSystemState(_)).WillOnce(Return(false));
        const auto status = collector.GetSystemState(state);
        ASSERT_THAT(status, Eq(false));
        TestIsOld(ElementId::State);
        TestIsOld(ElementId::Status);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetSystemStateSuccess)
    {
        State state;
        MockTelemetryLock();
        EXPECT_CALL(driver, GetSystemState(_)).WillOnce(Return(true));
        const auto status = collector.GetSystemState(state);
        ASSERT_THAT(status, Eq(true));
        TestIsNew(ElementId::State);
        TestIsNew(ElementId::Status);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetSystemStateLockFailure)
    {
        State state;
        MockTelemetryLockFailure();
        EXPECT_CALL(driver, GetSystemState(_)).WillOnce(Return(true));
        const auto status = collector.GetSystemState(state);
        ASSERT_THAT(status, Eq(true));
        TestIsOld(ElementId::State);
        TestIsOld(ElementId::Status);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetSystemStateTelemetry)
    {
        State element;
        MockTelemetryLock();
        EXPECT_CALL(driver, GetSystemState(_)).WillOnce(Invoke([](State& state) {
            state.anyParameterUpdatedSinceStartup = true;
            state.error = Error(0xf0);
            state.mode = Mode::Detumble;
            state.uptime = 10s;
            state.status = 0x0f;
            return true;
        }));

        collector.GetSystemState(element);

        TestIsNew(ElementId::State);
        TestIsNew(ElementId::Status);
        CaptureTelemetry();
        TestIsOld(ElementId::State);
        TestIsOld(ElementId::Status);
        const auto imtqStatus = state.Get<ImtqStatus>();
        ASSERT_THAT(imtqStatus.GetValue(), Eq(0x0f));
        const auto imtqState = state.Get<ImtqState>();
        ASSERT_THAT(imtqState.ConfigurationUpdated(), Eq(true));
        ASSERT_THAT(imtqState.ErrorCode(), Eq(0xf0));
        ASSERT_THAT(imtqState.Mode(), Eq(Mode::Detumble));
        ASSERT_THAT(imtqState.Uptime(), Eq(10s));
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetCalibratedMagnetometerDataFailure)
    {
        MagnetometerMeasurementResult state;
        EXPECT_CALL(driver, GetCalibratedMagnetometerData(_)).WillOnce(Return(false));
        const auto status = collector.GetCalibratedMagnetometerData(state);
        ASSERT_THAT(status, Eq(false));
        TestIsOld(ElementId::Magnetometer);
        TestIsOld(ElementId::CoilsActive);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetCalibratedMagnetometerDataSuccess)
    {
        MagnetometerMeasurementResult state;
        MockTelemetryLock();
        EXPECT_CALL(driver, GetCalibratedMagnetometerData(_)).WillOnce(Return(true));
        const auto status = collector.GetCalibratedMagnetometerData(state);
        ASSERT_THAT(status, Eq(true));
        TestIsNew(ElementId::Magnetometer);
        TestIsNew(ElementId::CoilsActive);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetCalibratedMagnetometerDataLockFailure)
    {
        MagnetometerMeasurementResult state;
        MockTelemetryLockFailure();
        EXPECT_CALL(driver, GetCalibratedMagnetometerData(_)).WillOnce(Return(true));
        const auto status = collector.GetCalibratedMagnetometerData(state);
        ASSERT_THAT(status, Eq(true));
        TestIsOld(ElementId::Magnetometer);
        TestIsOld(ElementId::CoilsActive);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetCalibratedMagnetometerDataTelemetry)
    {
        MagnetometerMeasurementResult element;
        MockTelemetryLock();
        EXPECT_CALL(driver, GetCalibratedMagnetometerData(_)).WillOnce(Invoke([](MagnetometerMeasurementResult& state) {
            state.coilActuationDuringMeasurement = true;
            state.data[0] = 1;
            state.data[1] = 2;
            state.data[2] = 3;
            return true;
        }));

        collector.GetCalibratedMagnetometerData(element);

        TestIsNew(ElementId::Magnetometer);
        TestIsNew(ElementId::CoilsActive);
        CaptureTelemetry();
        TestIsOld(ElementId::Magnetometer);
        TestIsOld(ElementId::CoilsActive);
        const auto imtqMag = state.Get<ImtqMagnetometerMeasurements>();
        ASSERT_THAT(imtqMag.GetValue()[0], Eq(1));
        ASSERT_THAT(imtqMag.GetValue()[1], Eq(2));
        ASSERT_THAT(imtqMag.GetValue()[2], Eq(3));
        const auto imtqcoils = state.Get<ImtqCoilsActive>();
        ASSERT_THAT(imtqcoils.GetValue(), Eq(true));
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetCoilCurrentFailure)
    {
        Vector3<Current> result;
        EXPECT_CALL(driver, GetCoilCurrent(_)).WillOnce(Return(false));
        const auto status = collector.GetCoilCurrent(result);
        ASSERT_THAT(status, Eq(false));
        TestIsOld(ElementId::CoilCurrents);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetCoilCurrentSuccess)
    {
        MockTelemetryLock();
        Vector3<Current> result;
        EXPECT_CALL(driver, GetCoilCurrent(_)).WillOnce(Return(true));
        const auto status = collector.GetCoilCurrent(result);
        ASSERT_THAT(status, Eq(true));
        TestIsNew(ElementId::CoilCurrents);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetCoilCurrentLockFailure)
    {
        MockTelemetryLockFailure();
        Vector3<Current> result;
        EXPECT_CALL(driver, GetCoilCurrent(_)).WillOnce(Return(true));
        const auto status = collector.GetCoilCurrent(result);
        ASSERT_THAT(status, Eq(true));
        TestIsOld(ElementId::CoilCurrents);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetCoilCurrentTelemetry)
    {
        MockTelemetryLock();
        Vector3<Current> result;
        EXPECT_CALL(driver, GetCoilCurrent(_)).WillOnce(Invoke([](auto& result) {
            int i = 0;
            for (auto& entry : result)
            {
                entry = i++;
            }

            return true;
        }));

        collector.GetCoilCurrent(result);

        ImtqCoilCurrent telemetry;
        Capture(telemetry, ElementId::CoilCurrents);
        int i = 0;
        for (auto entry : telemetry.GetValue())
        {
            EXPECT_THAT(entry, Eq(i));
            ++i;
        }
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetCoilTemperatureFailure)
    {
        Vector3<TemperatureMeasurement> result;
        EXPECT_CALL(driver, GetCoilTemperature(_)).WillOnce(Return(false));
        const auto status = collector.GetCoilTemperature(result);
        ASSERT_THAT(status, Eq(false));
        TestIsOld(ElementId::CoilTemperatures);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetCoilTemperatureSuccess)
    {
        MockTelemetryLock();
        Vector3<TemperatureMeasurement> result;
        EXPECT_CALL(driver, GetCoilTemperature(_)).WillOnce(Return(true));
        const auto status = collector.GetCoilTemperature(result);
        ASSERT_THAT(status, Eq(true));
        TestIsNew(ElementId::CoilTemperatures);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetCoilTemperatureLockFailure)
    {
        MockTelemetryLockFailure();
        Vector3<TemperatureMeasurement> result;
        EXPECT_CALL(driver, GetCoilTemperature(_)).WillOnce(Return(true));
        const auto status = collector.GetCoilTemperature(result);
        ASSERT_THAT(status, Eq(true));
        TestIsOld(ElementId::CoilTemperatures);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetCoilTemperatureTelemetry)
    {
        MockTelemetryLock();
        Vector3<TemperatureMeasurement> result;
        EXPECT_CALL(driver, GetCoilTemperature(_)).WillOnce(Invoke([](auto& result) {
            int i = 0;
            for (auto& entry : result)
            {
                entry = i++;
            }

            return true;
        }));

        collector.GetCoilTemperature(result);

        ImtqCoilTemperature telemetry;
        Capture(telemetry, ElementId::CoilTemperatures);
        int i = 0;
        for (auto entry : telemetry.GetValue())
        {
            EXPECT_THAT(entry, Eq(i));
            ++i;
        }
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetDetumbleDataFailure)
    {
        DetumbleData state;
        EXPECT_CALL(driver, GetDetumbleData(_)).WillOnce(Return(false));
        const auto status = collector.GetDetumbleData(state);
        ASSERT_THAT(status, Eq(false));
        TestIsOld(ElementId::Magnetometer);
        TestIsOld(ElementId::Bdot);
        TestIsOld(ElementId::Dipoles);
        TestIsOld(ElementId::CoilCurrents);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetDetumbleDataSuccess)
    {
        DetumbleData state;
        MockTelemetryLock();
        EXPECT_CALL(driver, GetDetumbleData(_)).WillOnce(Return(true));
        const auto status = collector.GetDetumbleData(state);
        ASSERT_THAT(status, Eq(true));
        TestIsNew(ElementId::Magnetometer);
        TestIsNew(ElementId::Bdot);
        TestIsNew(ElementId::Dipoles);
        TestIsNew(ElementId::CoilCurrents);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetDetumbleDataLockFailure)
    {
        DetumbleData state;
        MockTelemetryLockFailure();
        EXPECT_CALL(driver, GetDetumbleData(_)).WillOnce(Return(true));
        const auto status = collector.GetDetumbleData(state);
        ASSERT_THAT(status, Eq(true));
        TestIsOld(ElementId::Magnetometer);
        TestIsOld(ElementId::Bdot);
        TestIsOld(ElementId::Dipoles);
        TestIsOld(ElementId::CoilCurrents);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetDetumbleDataTelemetry)
    {
        DetumbleData element;
        MockTelemetryLock();
        EXPECT_CALL(driver, GetDetumbleData(_)).WillOnce(Invoke([](DetumbleData& state) {
            for (int i = 0; i < 3; ++i)
            {
                state.bDotData[i] = i;
                state.calibratedMagnetometerMeasurement[i] = i + 3;
                state.commandedCurrent[i] = i + 6;
                state.commandedDipole[i] = i + 9;
                state.filteredMagnetometerMeasurement[i] = i + 12;
                state.measuredCurrent[i] = i + 15;
            }

            return true;
        }));

        collector.GetDetumbleData(element);

        TestIsNew(ElementId::Magnetometer);
        TestIsNew(ElementId::Bdot);
        TestIsNew(ElementId::Dipoles);
        TestIsNew(ElementId::CoilCurrents);
        CaptureTelemetry();
        TestIsOld(ElementId::Magnetometer);
        TestIsOld(ElementId::Bdot);
        TestIsOld(ElementId::Dipoles);
        TestIsOld(ElementId::CoilCurrents);
        const auto imtqMag = state.Get<ImtqMagnetometerMeasurements>();
        ASSERT_THAT(imtqMag.GetValue()[0], Eq(3));
        ASSERT_THAT(imtqMag.GetValue()[1], Eq(4));
        ASSERT_THAT(imtqMag.GetValue()[2], Eq(5));

        const auto imtqBDot = state.Get<ImtqBDotTelemetry>();
        ASSERT_THAT(imtqBDot.GetValue()[0], Eq(0));
        ASSERT_THAT(imtqBDot.GetValue()[1], Eq(1));
        ASSERT_THAT(imtqBDot.GetValue()[2], Eq(2));

        const auto imtqDipoles = state.Get<ImtqDipoles>();
        ASSERT_THAT(imtqDipoles.GetValue()[0], Eq(9));
        ASSERT_THAT(imtqDipoles.GetValue()[1], Eq(10));
        ASSERT_THAT(imtqDipoles.GetValue()[2], Eq(11));

        const auto imtqCurrent = state.Get<ImtqCoilCurrent>();
        ASSERT_THAT(imtqCurrent.GetValue()[0], Eq(15));
        ASSERT_THAT(imtqCurrent.GetValue()[1], Eq(16));
        ASSERT_THAT(imtqCurrent.GetValue()[2], Eq(17));
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetHouseKeepingEngineeringFailure)
    {
        HouseKeepingEngineering state;
        EXPECT_CALL(driver, GetHouseKeepingEngineering(_)).WillOnce(Return(false));
        const auto status = collector.GetHouseKeepingEngineering(state);
        ASSERT_THAT(status, Eq(false));
        TestIsOld(ElementId::Status);
        TestIsOld(ElementId::HouseKeeping);
        TestIsOld(ElementId::CoilTemperatures);
        TestIsOld(ElementId::CoilCurrents);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetHouseKeepingEngineeringSuccess)
    {
        HouseKeepingEngineering state;
        MockTelemetryLock();
        EXPECT_CALL(driver, GetHouseKeepingEngineering(_)).WillOnce(Return(true));
        const auto status = collector.GetHouseKeepingEngineering(state);
        ASSERT_THAT(status, Eq(true));
        TestIsNew(ElementId::Status);
        TestIsNew(ElementId::HouseKeeping);
        TestIsNew(ElementId::CoilTemperatures);
        TestIsNew(ElementId::CoilCurrents);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetHouseKeepingEngineeringLockFailure)
    {
        HouseKeepingEngineering state;
        MockTelemetryLockFailure();
        EXPECT_CALL(driver, GetHouseKeepingEngineering(_)).WillOnce(Return(true));
        const auto status = collector.GetHouseKeepingEngineering(state);
        ASSERT_THAT(status, Eq(true));
        TestIsOld(ElementId::Status);
        TestIsOld(ElementId::HouseKeeping);
        TestIsOld(ElementId::CoilTemperatures);
        TestIsOld(ElementId::CoilCurrents);
    }

    TEST_F(ImtqTelemetryCollectorTest, TestGetHouseKeepingEngineeringTelemetry)
    {
        HouseKeepingEngineering element;
        MockTelemetryLock();
        EXPECT_CALL(driver, GetHouseKeepingEngineering(_)).WillOnce(Invoke([](HouseKeepingEngineering& state) {
            state.analogCurrent = 1;
            state.digitalCurrent = 2;
            state.analogVoltage = 3;
            state.digitalVoltage = 4;
            state.MCUtemperature = 5;
            state.status = 6;
            for (int i = 0; i < 3; ++i)
            {
                state.coilCurrent[i] = i + 7;
                state.coilTemperature[i] = i + 10;
            }

            return true;
        }));

        collector.GetHouseKeepingEngineering(element);

        TestIsNew(ElementId::Status);
        TestIsNew(ElementId::HouseKeeping);
        TestIsNew(ElementId::CoilTemperatures);
        TestIsNew(ElementId::CoilCurrents);
        CaptureTelemetry();
        TestIsOld(ElementId::Status);
        TestIsOld(ElementId::HouseKeeping);
        TestIsOld(ElementId::CoilTemperatures);
        TestIsOld(ElementId::CoilCurrents);
        const auto imtqHk = state.Get<ImtqHousekeeping>();
        ASSERT_THAT(imtqHk.AnalogCurrent(), Eq(1));
        ASSERT_THAT(imtqHk.DigitalCurrent(), Eq(2));
        ASSERT_THAT(imtqHk.AnalogVoltage(), Eq(3));
        ASSERT_THAT(imtqHk.DigitalVoltage(), Eq(4));
        ASSERT_THAT(imtqHk.McuTemperature(), Eq(5));

        const auto imtqStatus = state.Get<ImtqStatus>();
        ASSERT_THAT(imtqStatus.GetValue(), Eq(6));

        const auto imtqTemp = state.Get<ImtqCoilTemperature>();
        ASSERT_THAT(imtqTemp.GetValue()[0], Eq(10));
        ASSERT_THAT(imtqTemp.GetValue()[1], Eq(11));
        ASSERT_THAT(imtqTemp.GetValue()[2], Eq(12));

        const auto imtqCurrent = state.Get<ImtqCoilCurrent>();
        ASSERT_THAT(imtqCurrent.GetValue()[0], Eq(7));
        ASSERT_THAT(imtqCurrent.GetValue()[1], Eq(8));
        ASSERT_THAT(imtqCurrent.GetValue()[2], Eq(9));
    }
}
