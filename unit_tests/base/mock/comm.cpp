#include "comm.hpp"
#include "telecommunication/downlink.h"

using telecommunication::downlink::DownlinkAPID;

TransmitterMock::TransmitterMock()
{
}

TransmitterMock::~TransmitterMock()
{
}

void TransmitterMock::ExpectDownlinkFrame(DownlinkAPID apid, std::uint8_t correlationId, std::uint8_t errorCode)
{
    EXPECT_CALL(*this, SendFrame(IsDownlinkFrame(apid, 0, testing::ElementsAre(correlationId, errorCode))));
}

BeaconControllerMock::BeaconControllerMock()
{
}

BeaconControllerMock::~BeaconControllerMock()
{
}

CommTelemetryProviderMock::CommTelemetryProviderMock()
{
}

CommTelemetryProviderMock::~CommTelemetryProviderMock()
{
}

CommHardwareObserverMock::CommHardwareObserverMock()
{
}

CommHardwareObserverMock::~CommHardwareObserverMock()
{
}
