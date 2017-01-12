#include "downlink.h"
#include "utils.h"

namespace telecommunication
{
    static_assert(num(APID::LastItem) - 1 <= MaxValueOnBits(6), "APID is 6-bit value");

    DownlinkFrame::DownlinkFrame(APID apid, std::uint32_t seq) : _payloadWriter(gsl::make_span(_frame).subspan(3))
    {
        this->_frame.fill(0);

        Writer w(this->_frame);

        uint32_t header = (num(apid) << 18) | (seq & 0x3FFFF);
        w.WriteLowerBytesBE(header, 3);
    }
}
