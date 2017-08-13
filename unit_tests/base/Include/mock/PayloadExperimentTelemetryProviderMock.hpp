#ifndef UNIT_TESTS_BASE_INCLUDE_MOCK_PAYLOADEXPERIMENTTELEMETRYPROVIDERMOCK_HPP_
#define UNIT_TESTS_BASE_INCLUDE_MOCK_PAYLOADEXPERIMENTTELEMETRYPROVIDERMOCK_HPP_

#include "gmock/gmock.h"
#include "experiment/payload/PayloadExperimentTelemetryProvider.hpp"

struct PayloadExperimentTelemetryProviderMock : public experiment::payload::IPayloadExperimentTelemetryProvider
{
    PayloadExperimentTelemetryProviderMock();
    ~PayloadExperimentTelemetryProviderMock();

    MOCK_METHOD1(Save, void(experiments::fs::ExperimentFile& file));
};

#endif /* UNIT_TESTS_BASE_INCLUDE_MOCK_PAYLOADEXPERIMENTTELEMETRYPROVIDERMOCK_HPP_ */
