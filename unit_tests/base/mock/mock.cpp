#include "AdcsMocks.hpp"
#include "GyroMock.hpp"
#include "StorageAccessMock.hpp"
#include "eps.hpp"
#include "mock/HasStateMock.hpp"
#include "mock/ImtqTelemetryCollectorMock.hpp"
#include "mock/fm25w.hpp"
#include "mock/power.hpp"

GyroscopeMock::GyroscopeMock()
{
}

GyroscopeMock::~GyroscopeMock()
{
}

StorageAccessMock::StorageAccessMock()
{
}

StorageAccessMock::~StorageAccessMock()
{
}

AdcsCoordinatorMock::AdcsCoordinatorMock()
{
}

AdcsCoordinatorMock::~AdcsCoordinatorMock()
{
}

DetumblingMock::DetumblingMock()
{
}

DetumblingMock::~DetumblingMock()
{
}

EpsTelemetryProviderMock::EpsTelemetryProviderMock()
{
}

EpsTelemetryProviderMock::~EpsTelemetryProviderMock()
{
}

FM25WDriverMock::FM25WDriverMock()
{
}

FM25WDriverMock::~FM25WDriverMock()
{
}

PowerControlMock::PowerControlMock()
{
    ON_CALL(*this, PrimaryAntennaPower()).WillByDefault(testing::Return(Some(true)));
}

PowerControlMock::~PowerControlMock()
{
}

ImtqTelemetryCollectorMock::ImtqTelemetryCollectorMock()
{
}

ImtqTelemetryCollectorMock::~ImtqTelemetryCollectorMock()
{
}
