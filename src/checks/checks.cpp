#include "fs/ExperimentFile.hpp"
#include "telecommunication/downlink.h"

namespace
{
    static_assert(experiments::fs::ExperimentFile::PacketLength == telecommunication::downlink::CorrelatedDownlinkFrame::MaxPayloadSize,
        "Packet Length is not equal to downlink frame size");
}
