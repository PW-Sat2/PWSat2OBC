#ifndef LIBS_DRIVERS_COMM_COMM_FRAME_HPP
#define LIBS_DRIVERS_COMM_COMM_FRAME_HPP

#pragma once

#include "comm.hpp"

namespace devices
{
    namespace comm
    {
        /**
         * @brief This type describes single received frame.
         * @ingroup LowerCommDriver
         * The payload view may not contain entire frame size, it will contain as much of the frame content as
         * it was possible to fit in the buffer used for frame retrieval.
         */
        class Frame
        {
          public:
            /**
             * @brief ctor.
             */
            Frame();

            /**
             * @brief ctor.
             * @param[in] doppler Frame doppler frequency.
             * @param[in] rssi Received Signal Strength Indicator (RSSI) at the frame reception time.
             * @param[in] fullSize Full frame size.
             * @param[in] data span that contains entire received frame
             */
            Frame(std::uint16_t doppler, std::uint16_t rssi, std::uint16_t fullSize, gsl::span<std::uint8_t> data);

            /**
             * @brief Returns span that contains entire received frame.
             * @return Frame contents span
             */
            const gsl::span<std::uint8_t>& Payload() const;

            /**
             * @brief Returns Current frame size.
             * @return Frame size in bytes.
             */
            std::uint16_t Size() const;

            /**
             * @brief Returns Actual frame size.
             * @return Frame size in bytes.
             */
            std::uint16_t FullSize() const;

            /**
             * @brief Returns frame doppler frequency.
             * @return Frame doppler frequency.
             */
            std::uint16_t Doppler() const;

            /**
             * @brief Return frame Received Signal Strength Indicator.
             * @return Received Signal Strength Indicator.
             */
            std::uint16_t Rssi() const;

            /**
             * @brief Verifies value of the doppler frequency for this frame.
             * @return Verification status.
             * @retval true Doppler frequency is valid.
             * @retval false Doppler frequency is invalid.
             */
            bool IsDopplerValid() const;

            /**
             * @brief Verifies value of the rssi for this frame.
             * @return Verification status.
             * @retval true Rssi is valid.
             * @retval false Rssi is invalid.
             */
            bool IsRssiValid() const;

            /**
             * @brief Verifies size of this frame.
             * @return Verification status.
             * @retval true Frame size is valid.
             * @retval false Frame size is invalid.
             */
            bool IsSizeValid() const;

            /**
             * @brief Verifies whether the entire frame content is represented by this object.
             * @return Verification status.
             * @retval true Entire frame is available.
             * @retval false At least one payload byte is missing.
             */
            bool IsComplete() const;

            /**
             * @brief Verifies state of this frame object.
             * @return Verification status.
             * @retval true There are no errors and entire frame payload is available.
             * @retval false There are errors or there is at least one byte missing.
             */
            bool Verify() const;

          private:
            /** @brief Doppler frequency. This field contains the measured Doppler shift on the packet at the reception time. */
            std::uint16_t doppler;

            /** @brief This field contains the measured Received Signal Strength Indicator (RSSI) at the reception time. */
            std::uint16_t rssi;

            /** @brief Complete size of the current frame. */
            std::uint16_t fullFrameSize;

            /** @brief Frame content. */
            gsl::span<std::uint8_t> content;
        };

        inline const gsl::span<uint8_t>& Frame::Payload() const
        {
            return this->content;
        }

        inline std::uint16_t Frame::Size() const
        {
            return gsl::narrow_cast<std::uint16_t>(this->content.size());
        }

        inline std::uint16_t Frame::FullSize() const
        {
            return this->fullFrameSize;
        }

        inline std::uint16_t Frame::Doppler() const
        {
            return this->doppler;
        }

        inline std::uint16_t Frame::Rssi() const
        {
            return this->rssi;
        }

        inline bool Frame::IsDopplerValid() const
        {
            return (this->doppler & 0xf000) == 0;
        }

        inline bool Frame::IsRssiValid() const
        {
            return (this->rssi & 0xf000) == 0;
        }

        inline bool Frame::IsSizeValid() const
        {
            return this->fullFrameSize <= MaxFrameSize;
        }

        inline bool Frame::IsComplete() const
        {
            return this->fullFrameSize == gsl::narrow_cast<std::uint16_t>(this->content.size());
        }

        inline bool Frame::Verify() const
        {
            return IsDopplerValid() && IsRssiValid() && IsSizeValid() && IsComplete();
        }
    }
}
#endif
