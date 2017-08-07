#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "OsMock.hpp"
#include "mock/ImtqDriverMock.hpp"
#include "os/os.hpp"
#include "telemetry/ImtqTelemetryCollector.hpp"
#include "telemetry/state.hpp"

namespace
{
    using testing::Eq;
    using testing::Return;

    class ImtqTelemetryCollectorTest : public ::testing::Test
    {
      protected:
        ImtqTelemetryCollectorTest();

        ImtqDriverMock driver;
        devices::imtq::ImtqTelemetryCollector collector;
        telemetry::ManagedTelemetry state;
        testing::NiceMock<OSMock> os;
        OSReset osReset;
    };

    ImtqTelemetryCollectorTest::ImtqTelemetryCollectorTest() : collector(driver)
    {
        osReset = InstallProxy(&os);
    }
}
