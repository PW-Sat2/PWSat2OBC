#ifndef UNIT_TESTS_BASE_INCLUDE_MOCK_PAYLOADDEVICEMOCK_HPP_
#define UNIT_TESTS_BASE_INCLUDE_MOCK_PAYLOADDEVICEMOCK_HPP_

#include "gmock/gmock.h"
#include "payload/interfaces.h"

struct PayloadDeviceMock : public devices::payload::IPayloadDeviceDriver
{
    PayloadDeviceMock();
    ~PayloadDeviceMock();

    MOCK_METHOD1(MeasureSunSRef, OSResult(devices::payload::PayloadTelemetry::SunsRef& output));
    MOCK_METHOD1(MeasureTemperatures, OSResult(devices::payload::PayloadTelemetry::Temperatures& output));
    MOCK_METHOD1(MeasurePhotodiodes, OSResult(devices::payload::PayloadTelemetry::Photodiodes& output));
    MOCK_METHOD1(MeasureHousekeeping, OSResult(devices::payload::PayloadTelemetry::Housekeeping& output));
    MOCK_METHOD1(RadFETOn, OSResult(devices::payload::PayloadTelemetry::Radfet& output));
    MOCK_METHOD1(MeasureRadFET, OSResult(devices::payload::PayloadTelemetry::Radfet& output));
    MOCK_METHOD1(RadFETOff, OSResult(devices::payload::PayloadTelemetry::Radfet& output));
    MOCK_METHOD1(GetWhoami, OSResult(devices::payload::PayloadTelemetry::Status& output));
    MOCK_CONST_METHOD1(ValidateWhoami, bool(const devices::payload::PayloadTelemetry::Status& data));
};

#endif /* UNIT_TESTS_BASE_INCLUDE_MOCK_PAYLOADDEVICEMOCK_HPP_ */
