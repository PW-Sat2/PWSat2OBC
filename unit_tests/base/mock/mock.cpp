#include "AdcsMocks.hpp"
#include "GyroMock.hpp"
#include "PayloadDeviceMock.hpp"
#include "StorageAccessMock.hpp"
#include "eps.hpp"
#include "mock/DeploySolarArrayMock.hpp"
#include "mock/HasStateMock.hpp"
#include "mock/ImtqDriverMock.hpp"
#include "mock/ImtqTelemetryCollectorMock.hpp"
#include "mock/LineIOMock.hpp"
#include "mock/OpenSailMock.hpp"
#include "mock/PhotoServiceMock.hpp"
#include "mock/PayloadExperimentTelemetryProviderMock.hpp"
#include "mock/TemperatureReaderMock.hpp"
#include "mock/experiment.hpp"
#include "mock/fm25w.hpp"
#include "mock/n25q.hpp"
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

ImtqDriverMock::ImtqDriverMock()
{
}

ImtqDriverMock::~ImtqDriverMock()
{
}

ImtqTelemetryCollectorMock::ImtqTelemetryCollectorMock()
{
}

ImtqTelemetryCollectorMock::~ImtqTelemetryCollectorMock()
{
}

ExperimentControllerMock::ExperimentControllerMock()
{
}

ExperimentControllerMock::~ExperimentControllerMock()
{
}

PayloadDeviceMock::PayloadDeviceMock()
{
}

PayloadDeviceMock::~PayloadDeviceMock()
{
}

N25QDriverMock::N25QDriverMock()
{
}

N25QDriverMock::~N25QDriverMock()
{
}

LineIOMock::LineIOMock()
{
}

LineIOMock::~LineIOMock()
{
}

PhotoServiceMock::PhotoServiceMock()
{
}

PhotoServiceMock::~PhotoServiceMock()
{
}

OpenSailMock::OpenSailMock()
{
}

OpenSailMock::~OpenSailMock()
{
}

DeploySolarArrayMock::DeploySolarArrayMock()
{
}

DeploySolarArrayMock::~DeploySolarArrayMock()
{
}

PayloadExperimentTelemetryProviderMock::PayloadExperimentTelemetryProviderMock()
{
}
PayloadExperimentTelemetryProviderMock::~PayloadExperimentTelemetryProviderMock()
{
}

TemperatureReaderMock::TemperatureReaderMock()
{
}
TemperatureReaderMock::~TemperatureReaderMock()
{
}
