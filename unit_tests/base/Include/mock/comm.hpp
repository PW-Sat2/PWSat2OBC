#ifndef UNIT_TESTS_MOCK_COMM_HPP_
#define UNIT_TESTS_MOCK_COMM_HPP_

#include <cstdint>
#include "gmock/gmock.h"
#include "comm/Beacon.hpp"
#include "comm/CommTelemetry.hpp"
#include "comm/IBeaconController.hpp"
#include "comm/ITransmitter.hpp"
#include "comm/comm.hpp"
#include "gsl/span"
#include "telecommunication/downlink.h"

struct TransmitterMock : public devices::comm::ITransmitter
{
    TransmitterMock();
    ~TransmitterMock();
    MOCK_METHOD1(SendFrame, bool(gsl::span<const std::uint8_t>));
    MOCK_METHOD1(GetTransmitterTelemetry, bool(devices::comm::TransmitterTelemetry&));
    MOCK_METHOD1(SetTransmitterStateWhenIdle, bool(devices::comm::IdleState));
    MOCK_METHOD1(SetTransmitterBitRate, bool(devices::comm::Bitrate));
    MOCK_METHOD1(GetTransmitterState, bool(devices::comm::TransmitterState&));
    MOCK_METHOD0(ResetTransmitter, bool());
};

struct BeaconControllerMock : public devices::comm::IBeaconController
{
    BeaconControllerMock();
    ~BeaconControllerMock();
    MOCK_METHOD1(SetBeacon, Option<bool>(const devices::comm::Beacon& beacon));
    MOCK_METHOD0(ClearBeacon, bool());
};

struct CommTelemetryProviderMock : public devices::comm::ICommTelemetryProvider
{
    CommTelemetryProviderMock();
    ~CommTelemetryProviderMock();
    MOCK_METHOD1(GetTelemetry, bool(devices::comm::CommTelemetry& telemetry));
};

struct CommHardwareObserverMock : public devices::comm::ICommHardwareObserver
{
    CommHardwareObserverMock();
    ~CommHardwareObserverMock();
    MOCK_METHOD0(WaitForComLoop, void());
};

MATCHER_P3(IsDownlinkFrame, apidMatcher, seqMatcher, payloadMatcher, "")
{
    auto num = arg[0] | (arg[1] << 8) | (arg[2] << 16);

    auto apid = static_cast<telecommunication::downlink::DownlinkAPID>(num & 0x3F);
    std::uint32_t seq = (num >> 6) & 0x3FFFF;
    auto payload = arg.subspan(3);

    return testing::Matches(apidMatcher)(apid) //
        && testing::Matches(seqMatcher)(seq)   //
        && testing::Matches(payloadMatcher)(payload);
    ;
}

#endif /* UNIT_TESTS_MOCK_COMM_HPP_ */
