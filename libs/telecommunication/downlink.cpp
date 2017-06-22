#include "downlink.h"
#include "base/BitWriter.hpp"
#include "utils.h"

namespace telecommunication
{
    namespace downlink
    {
        static_assert(num(DownlinkAPID::LastItem) - 1 <= MaxValueOnBits(6), "APID is 6-bit value");

        RawFrame::RawFrame() : _payloadWriter(gsl::make_span(_frame))
        {
        }

        DownlinkFrame::DownlinkFrame(DownlinkAPID apid, std::uint32_t seq) : _payloadWriter(gsl::make_span(_frame).subspan(3))
        {
            this->_frame.fill(0);

            BitWriter w(this->_frame);
            w.WriteWord(num(apid), 6);
            w.WriteDoubleWord(seq, 18);
        }

        CorrelatedDownlinkFrame::CorrelatedDownlinkFrame(DownlinkAPID apid, std::uint32_t seq, std::uint8_t correlationId)
            : DownlinkFrame(apid, seq)
        {
            PayloadWriter().WriteByte(correlationId);
        }
    }
}
