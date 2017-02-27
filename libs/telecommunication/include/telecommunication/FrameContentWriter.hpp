#ifndef LIBS_TELECOMMUNICATION_INCLUDE_TELECOMMUNICATION_FRAME_CONTENT_WRITER_HPP
#define LIBS_TELECOMMUNICATION_INCLUDE_TELECOMMUNICATION_FRAME_CONTENT_WRITER_HPP

#pragma once

#include <type_traits>
#include "base/writer.h"
#include "system.h"

namespace telecommunication
{
    namespace downlink
    {
        /**
         * @ingroup telecomm_handling
         * @{
         */

        /**
         * @brief Enumerator of all possible fields that are stored inside the downlink frame.
         */
        enum class FieldId : std::uint8_t
        {
            None = 0,     //!< No field.
            TimeStamp = 1 //!< Field that contains time value (64 bit integer with time since mission start in ms)
        };

        /**
         * @brief This class is an adapter over the standard buffer Writer that ties together field identifiers
         * with their values.
         *
         * @see Writer for details
         */
        class FrameContentWriter final
        {
          public:
            /**
             * @brief ctor.
             * @param[in] dataWriter Reference to standard writer that should be used for data serialization.
             */
            FrameContentWriter(Writer& dataWriter);

            /**
             * @brief Appends single byte to the buffer and moves the current position to the next byte.
             * @param[in] fieldId Identifier of the stored field.
             * @param[in] byte Byte that should be added to writer output.
             * @return Operation status.
             */
            bool WriteByte(FieldId fieldId, std::uint8_t byte);

            /**
             * @brief Writes single 16 bit word with little-endian memory orientation to the buffer
             * and advances the current buffer position to the next unused byte.
             * @param[in] fieldId Identifier of the stored field.
             * @param[in] word Word that should be added to writer output using little endian byte ordering.
             * @return Operation status.
             */
            bool WriteWordLE(FieldId fieldId, std::uint16_t word);

            /**
             * @brief Writes single 32 bit word with little-endian memory orientation to the buffer
             * and advances the current buffer position to the next unused byte.
             * @param[in] fieldId Identifier of the stored field.
             * @param[in] dword Doubleword that should be added to writer output using little endian byte ordering.
             * @return Operation status.
             */
            bool WriteDoubleWordLE(FieldId fieldId, std::uint32_t dword);

            /**
             * @brief Writes single 64 bit word with little-endian memory orientation to the buffer
             * and advances the current buffer position to the next unused byte.
             * @param[in] fieldId Identifier of the stored field.
             * @param[in] qword Quadword that should be added to writer output using little endian byte ordering.
             * @return Operation status.
             */
            bool WriteQuadWordLE(FieldId fieldId, std::uint64_t qword);

            /**
             * @brief Writes single 16 bit signed value with little-endian memory orientation to the buffer
             * and advances the current buffer position to the next unused byte.
             * Value is written in 2's complement notation.
             * @param[in] fieldId Identifier of the stored field.
             * @param[in] word Word that should be added to writer output using little endian byte ordering.
             * @return Operation status.
             */
            bool WriteSignedWordLE(FieldId fieldId, std::int16_t word);

            /**
             * @brief Writes single 32 bit signed value with little-endian memory orientation to the buffer
             * and advances the current buffer position to the next unused byte.
             * Value is written in 2's complement notation.
             * @param[in] fieldId Identifier of the stored field.
             * @param[in] dword Doubleword that should be added to writer output using little endian byte ordering.
             * @return Operation status.
             */
            bool WriteSignedDoubleWordLE(FieldId fieldId, std::int32_t dword);

            /**
             * @brief Writes the requested memory block to the buffer.
             *
             * @param[in] fieldId Identifier of the stored field.
             * @param[in] buffer Span whose contents should be appended to writer output.
             * @return Operation status.
             */
            bool WriteArray(FieldId fieldId, gsl::span<const std::uint8_t> buffer);

            /**
             * @brief Resets reader to the initial state.
             */
            void Reset();

          private:
            /**
             * @brief Saves field idenfier to stream.
             * @param[in] fieldId Field identifier that should be stored.
             * @return Operation status.
             */
            bool WriteFieldId(FieldId fieldId);

            Writer& writer;
        };

        inline bool FrameContentWriter::WriteByte(FieldId fieldId, std::uint8_t byte)
        {
            WriteFieldId(fieldId);
            return writer.WriteByte(byte);
        }

        inline bool FrameContentWriter::WriteWordLE(FieldId fieldId, std::uint16_t word)
        {
            WriteFieldId(fieldId);
            return writer.WriteWordLE(word);
        }

        inline bool FrameContentWriter::WriteDoubleWordLE(FieldId fieldId, std::uint32_t dword)
        {
            WriteFieldId(fieldId);
            return writer.WriteDoubleWordLE(dword);
        }

        inline bool FrameContentWriter::WriteQuadWordLE(FieldId fieldId, std::uint64_t qword)
        {
            WriteFieldId(fieldId);
            return writer.WriteQuadWordLE(qword);
        }

        inline bool FrameContentWriter::WriteSignedWordLE(FieldId fieldId, std::int16_t word)
        {
            WriteFieldId(fieldId);
            return writer.WriteSignedWordLE(word);
        }

        inline bool FrameContentWriter::WriteSignedDoubleWordLE(FieldId fieldId, std::int32_t dword)
        {
            WriteFieldId(fieldId);
            return writer.WriteSignedDoubleWordLE(dword);
        }

        inline bool FrameContentWriter::WriteArray(FieldId fieldId, gsl::span<const std::uint8_t> buffer)
        {
            WriteFieldId(fieldId);
            return writer.WriteArray(buffer);
        }

        inline bool FrameContentWriter::WriteFieldId(FieldId fieldId)
        {
            return writer.WriteByte(num(fieldId));
        }

        inline void FrameContentWriter::Reset()
        {
            this->writer.Reset();
        }
    }
}

#endif
