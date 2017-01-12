#ifndef LIBS_TELECOMMUNICATION_INCLUDE_TELECOMMUNICATION_DOWNLINK_H_
#define LIBS_TELECOMMUNICATION_INCLUDE_TELECOMMUNICATION_DOWNLINK_H_

#include <cstdint>
#include "base/writer.h"
#include "comm/comm.hpp"
#include "gsl/span"

namespace telecommunication
{
    enum class APID : std::uint8_t
    {
        TelemetryShort = 0x2A,
        TelemetryLong = 0x3F,
        LastItem
    };

    class DownlinkFrame final
    {
      public:
        DownlinkFrame(APID apid, std::uint32_t seq);

        inline Writer& PayloadWriter();
        inline gsl::span<uint8_t> Frame();

        static constexpr std::uint8_t HeaderSize = 3;
        static constexpr std::uint8_t MaxPayloadSize = devices::comm::MaxDownlinkFrameSize - HeaderSize;

      private:
        std::array<uint8_t, devices::comm::MaxDownlinkFrameSize> _frame;
        Writer _payloadWriter;
    };

    inline Writer& DownlinkFrame::PayloadWriter()
    {
        return this->_payloadWriter;
    }

    inline gsl::span<uint8_t> DownlinkFrame::Frame()
    {
        gsl::span<uint8_t> s(this->_frame);

        return s.subspan(0, 3 + this->_payloadWriter.GetDataLength());
    }
}

#endif /* LIBS_TELECOMMUNICATION_INCLUDE_TELECOMMUNICATION_DOWNLINK_H_ */
