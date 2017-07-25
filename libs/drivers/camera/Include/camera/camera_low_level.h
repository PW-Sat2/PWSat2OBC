#ifndef _CAMERA_LOW_LEVEL_H_
#define _CAMERA_LOW_LEVEL_H_

#include <chrono>
#include <cstdint>
#include "camera_types.h"
#include "command_factory.h"
#include "gsl/span"
#include "line_io.h"
#include "uart/uart.hpp"

namespace devices
{
    namespace camera
    {
        /** @brief Class that implements low level camera commands */
        class LowLevelCameraDriver
        {
          public:
            /**
             * @brief Constructs Low Level Camera driver instance
             * @param lineIO Line interface to use
             */
            LowLevelCameraDriver(LineIO& lineIO);

            /**
             * @brief Sends SNAPSHOT command
             * @param type Camera snapshot type
             * @return False if failed to ACK command, true if everything is ok
             */
            bool SendSnapshot(CameraSnapshotType type);

            /**
             * @brief Sends Jpeg INITIAL command
             * @param jpegResolution Jpeg resolution
             * @return False if failed to ACK command, true if everything is ok
             */
            bool SendJPEGInitial(CameraJPEGResolution jpegResolution);

            /**
             * @brief Sends SET PACKAGE SIZE command
             * @param packageSize package size in bytes, must be greater or equal 64 and smaller or equal 512
             * @return False if failed to ACK command or if package size is invalid, true if everything is ok
             */
            bool SendSetPackageSize(uint16_t packageSize);

            /**
             * @brief Sends SYNC command.
             * @param timeout Timeout for sync command.
             * @return False if failed to read and parse ACK, true if everything is ok
             */
            bool SendSync(std::chrono::milliseconds timeout);

            /**
             * @brief Sends RESET command.
             * @return False if failed to ACK command, true if everything is ok
             */
            bool SendReset();

            /**
             * @brief Build and send ack for getting data or picture command
             * @param cmdAck camera command structure
             * @param packageIdLow Lowest part of package id
             * @param packageIdHigh Highest part of package id
             */
            void SendAck(CameraCmd cmdAck, uint8_t packageIdLow = 0, uint8_t packageIdHigh = 0);

            /**
             * @brief Sends GET PICTURE command.
             * @param type Type of picture
             * @param pictureData Instance that will hold picture data if command succeeds.
             * @return False when fail to read or parse response, true if everything is ok
             */
            bool SendGetPictureJPEG(CameraPictureType::Enum type, PictureData& pictureData);

            /**
             * @brief Sends SET BAUD RATE command.
             * @param firstDivider First divider
             * @param secondDivider Second divider
             * @return False when failed to ACK the command, true otherwise.
             */
            bool SendSetBaudRate(uint8_t firstDivider, uint8_t secondDivider);

            /**
             * @brief Sends ACK while expecting response.
             * @param ackedCommand Command to ackowledge
             * @param packageIdLow Lower byte of package ID to ackowledge
             * @param packageIdHigh Higher byte of package ID to ackowledge
             * @param receiveBuffer Buffer where response will be written
             * @param expectedBytes Number of bytes expected to be received
             * @param timeout Timeout when reading line
             * @return False if read timeouts or not all bytes were read, true if everything is ok
             */
            bool SendAckWithResponse(                                //
                CameraCmd ackedCommand,                              //
                uint8_t packageIdLow,                                //
                uint8_t packageIdHigh,                               //
                gsl::span<uint8_t> receiveBuffer,                    //
                uint8_t expectedBytes,                               //
                std::chrono::milliseconds timeout = DefaultTimeout); //

          private:
            devices::camera::CommandFactory _commandFactory;
            LineIO& _lineIO;

            void LogSendCommand(gsl::span<uint8_t> cmd);

            void LogReceivedCommand(gsl::span<const uint8_t> cmd);

            bool IsValidSyncResponse(gsl::span<const uint8_t> cmd);

            bool SendCommand(                                        //
                gsl::span<uint8_t> command,                          //
                gsl::span<uint8_t> receiveBuffer,                    //
                uint8_t additionalBytes,                             //
                std::chrono::milliseconds timeout = DefaultTimeout); //

            bool SendCommand(                                        //
                gsl::span<uint8_t> command,                          //
                std::chrono::milliseconds timeout = DefaultTimeout); //

            static constexpr std::chrono::milliseconds DefaultTimeout = std::chrono::seconds(5);

            static constexpr std::chrono::milliseconds ResetTimeout = std::chrono::milliseconds(100);
        };
    }
}
#endif /* _CAMERA_LOW_LEVEL_H_ */
