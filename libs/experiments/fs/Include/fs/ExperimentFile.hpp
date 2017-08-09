#ifndef LIBS_FS_INCLUDE_FS_TRANSPORTSTREAM_HPP_
#define LIBS_FS_INCLUDE_FS_TRANSPORTSTREAM_HPP_

#include <gsl/span>
#include "base/os.h"
#include "base/writer.h"
#include "fs/fs.h"
#include "time/ICurrentTime.hpp"

namespace experiments
{
    namespace fs
    {
        /**
         * @brief Container file for experiment results
         * @ingroup experiments
         *
         * This container is bit similar in principles to MPEG TS format.
         * It is designed to have:
         * - fixed packed length - it have to fit the downlink frame
         * - immune to data loss - one packet dropped should not invalidate entire stream
         * - any data type have to fit in it and container should not be aware of the contents
         * - data should not be divided by downlink frames
         * - be as compact as possible.
         *
         * Format:
         * byte 0 - PID::Synchronization - 8b
         * byte 1 - [optional] PID::Timestamp - 8b
         * byte 2 - [optional] timestamp - 64b
         * byte 10 - PID of payload data - 8b
         * byte 11 - payload data - variable length
         * var - PID of another payload data
         * ...
         * near packet end - PID::Padding - 8b
         * until the end - padding data (0xFF)
         */
        class ExperimentFile final
        {
          public:
            /** @brief Data packet length.  */
            static constexpr uint16_t PacketLength = 231;

            /** @brief Packet Id. */
            enum class PID : std::uint8_t
            {
                Reserved = 0,
                Timestamp = 1, // 8 bytes of data
                Error = 2,     // 8 bytes of data
                /* 3-16 reserved for control codes */
                Gyro = 0x10,                      // 8 bytes of data
                ExperimentalSunSPrimary = 0x11,   // Experimental SunS primary data, 41 bytes of data
                ExperimentalSunSSecondary = 0x12, // Experimental SunS primary data, 26 bytes of data
                ReferenceSunS = 0x13,             // Reference SunS data, 10 bytes of data
                RadFETStart = 0x14,               // RadFET data, 17 bytes
                RadFETEnd = 0x15,                 // RadFET data, 17 bytes
                RadFET = 0x16,                    // RadFET data, 17 bytes
                Temperature = 0x17,               // Temperature data, 4 bytes
                Sail = 0x18,                      // 3 bytes of data

                PayloadWhoami = 0x20,       // 1 byte of data
                PayloadSunS = 0x21,         // 10 bytes of data
                PayloadTemperatures = 0x22, // 18 bytes of data
                PayloadPhotodiodes = 0x23,  // 8 bytes of data
                PayloadHousekeeping = 0x24, // 4 bytes of data
                PayloadRadFet = 0x25,       // 17 bytes of data

                Synchronization = 0x47, // Synchronization PID indicating start of packet. 0 bytes of data.

                Continuation = 0xFE, // Indicates that following data are continuation of previous packet, not the new data.
                                     // Used when data size is greater than packet size.
                Padding = 0xFF       // use last all-one binary number. Data is 0xFF till the end.
            };

            /**
             * The data used in Padding block. It is not the same as Padding PID, as it may have length other than 8 bits.
             * Padding data is always 0xFF.
             */
            static constexpr uint8_t PaddingData = 0xFF;

            /**
             * @brief Default constructor
             * @param time Optional time provider. If set, each packet automatically have a timestamp.
             * */
            ExperimentFile(services::time::ICurrentTime* time = nullptr);

            /**
             * @brief Move ctor
             * @param other Other object
             */
            ExperimentFile(ExperimentFile&& other) = default;

            /**
             * @brief Move assignment operator
             * @param other Other object
             * @return Reference to this object
             */
            ExperimentFile& operator=(ExperimentFile&& other) = default;

            /**
             * @brief Destructor
             */
            ~ExperimentFile();

            /**
             * @brief Factory method that opens experiment file
             * @param fs File system
             * @param path File path
             * @param mode Open mode
             * @param access Access
             * @return ExperimentFile instance
             */
            bool Open(services::fs::IFileSystem& fs, const char* path, services::fs::FileOpen mode, services::fs::FileAccess access);

            /**
             * @brief Writes data to file.
             * @param pid The Packet Identifier of provided data.
             * @param data The data to write.
             * @returns Status of operation.
             */
            OSResult Write(PID pid, const gsl::span<uint8_t>& data);

            /**
             * @brief Closes the file.
             * @returns Status of operation.
             */
            OSResult Close();

            /**
             * @brief Flushes current packet to file system and begins new packet.
             * @returns Status of operation.
             */
            OSResult Flush();

          private:
            /** @brief The byte size of PID type.  */
            static constexpr size_t PIDSize = sizeof(PID);

            void FillBufferWithPadding();
            void InitializePacket();

            OSResult WriteDataBiggerThanFrame(PID pid, const gsl::span<uint8_t>& data);

            /** @brief Buffer for data.  */
            std::array<uint8_t, PacketLength> _buffer;

            /** @brief File that data will be saved to. */
            services::fs::File _file;

            /** @brief Time provider. */
            services::time::ICurrentTime* _time;

            Writer _writer;
            bool _hasPayloadInFrame;
        };
    }
}

#endif /* LIBS_FS_INCLUDE_FS_TRANSPORTSTREAM_HPP_ */
