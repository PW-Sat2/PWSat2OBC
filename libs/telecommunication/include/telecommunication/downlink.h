#ifndef LIBS_TELECOMMUNICATION_INCLUDE_TELECOMMUNICATION_DOWNLINK_H_
#define LIBS_TELECOMMUNICATION_INCLUDE_TELECOMMUNICATION_DOWNLINK_H_

#include <cstdint>
#include "base/writer.h"
#include "comm/comm.hpp"
#include "gsl/span"

namespace telecommunication
{
    namespace downlink
    {
        /**
         * @ingroup telecomm_handling
         * @{
         */

        /**
         * @brief Downlink APID definition
         *
         * @remark All values are 6-bit
         * @remark @ref DownlinkAPID::LastItem must be last item on the enum list
         */
        enum class DownlinkAPID : std::uint8_t
        {
            Pong = 0x01,           //!< Pong
            Operation = 0x2,       //!< Operation command
            Beacon = 0x3,          //!< Beacon
            TelemetryShort = 0x2A, //!< TelemetryShort
            TelemetryLong = 0x3F,  //!< TelemetryLong
            LastItem               //!< LastItem
        };

        /**
         * @brief Downlink frame implementation
         */
        class DownlinkFrame final
        {
          public:
            /**
             * @brief Initializes new @ref DownlinkFrame instance
             * @param apid APID
             * @param seq Sequence number
             */
            DownlinkFrame(DownlinkAPID apid, std::uint32_t seq);

            /**
             * @brief Returns writer that can be used to fill payload part of frame
             * @return Writer
             */
            inline Writer& PayloadWriter();

            /**
             * @brief Returns undelying byte representation of the frame
             * @return
             */
            inline gsl::span<uint8_t> Frame();

            /** @brief Size of header size */
            static constexpr std::uint8_t HeaderSize = 3;
            /** @brief Maximum size of payload inside single frame */
            static constexpr std::uint8_t MaxPayloadSize = devices::comm::MaxDownlinkFrameSize - HeaderSize;

          private:
            /** @brief Buffer in which frame is built */
            std::array<uint8_t, devices::comm::MaxDownlinkFrameSize> _frame;

            /** @brief Writer instance used to build frame payload */
            Writer _payloadWriter;
        };

        inline Writer& DownlinkFrame::PayloadWriter()
        {
            return this->_payloadWriter;
        }

        inline gsl::span<uint8_t> DownlinkFrame::Frame()
        {
            return gsl::make_span(this->_frame).subspan(0, HeaderSize + this->_payloadWriter.GetDataLength());
        }

        /** @} */
    }
}

#endif /* LIBS_TELECOMMUNICATION_INCLUDE_TELECOMMUNICATION_DOWNLINK_H_ */
