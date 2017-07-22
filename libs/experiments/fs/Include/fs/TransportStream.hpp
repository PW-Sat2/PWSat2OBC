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
        class TransportStream
        {
          public:
            /** @brief Data packet length.  */
            static constexpr uint16_t PacketLength = 232;

            /** @brief Packet Id. */
            enum class PID : std::uint8_t
            {
                Reserved = 0,
                Timestamp = 1, // 8 bytes of data
                /* 2-16 reserved for control codes */
                Gyro = 0x10, // 8 bytes of data

                Synchronization = 0x47, // Synchronisation PID indicating start of packet. 0 bytes of data.

                Padding = 0xFF // use last all-one binary number. Data is 0xFF till the end.
            };

            /**
             * @brief Default constrctor
             * @param time Optional time provider. If set, each packet automatically has timestamp.
             * */
            TransportStream(services::time::ICurrentTime* time = nullptr);

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
             */
            OSResult Write(PID pid, const gsl::span<uint8_t>& data);

            /**
             * @brief Closes the file.
             */
            OSResult Close();

            /**
             * @brief Flushes current packet to file system and begins new packet.
             */
            OSResult Flush();

          private:
            /** @brief The byte size of PID type.  */
            static constexpr size_t PIDSize = sizeof(PID);

            /** @brief Buffer for data.  */
            std::array<uint8_t, PacketLength> _buffer;

            /** @brief File that data will be saved to. */
            services::fs::File _file;

            /** @brief Time provider. */
            services::time::ICurrentTime* _time;

            Writer _writer;

            void FillBufferWithPadding();
            void InitializePacket();
        };
    }
}

#endif /* LIBS_FS_INCLUDE_FS_TRANSPORTSTREAM_HPP_ */
