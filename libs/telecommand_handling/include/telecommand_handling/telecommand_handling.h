#ifndef LIBS_TELECOMMAND_HANDLING_INCLUDE_TELECOMMAND_HANDLING_TELECOMMAND_HANDLING_H_
#define LIBS_TELECOMMAND_HANDLING_INCLUDE_TELECOMMAND_HANDLING_TELECOMMAND_HANDLING_H_

#include <cstdint>
#include <gsl/span>
#include "comm/comm.h"
#include "system.h"

namespace telecommands
{
    namespace handling
    {
        /**
         * @defgroup telecommand_handling Telecommand handling
         *
         * @brief Library that provides infrastructure for handling telecommands in frames received from @ref LowerCommDriver module
         *
         * @{
         */

        /** @brief Result of telecommand decryption */
        enum class DecryptStatus
        {
            /** Decryption successful */
            Success,

            /** Decryption failed */
            Failed
        };

        /**
         * @brief Result of telecommand decoding
         */
        enum class DecodeFrameStatus
        {
            /** Frame successfully decoded into telecommand */
            Success,

            /** Decoding frame failed */
            Failed
        };

        /**
         * Interface describing how incoming frame decryption should be performed
         */
        struct IDecryptFrame
        {
            /**
             * @brief Decrypts incoming frame
             * @param[in] frame Incoming (encrypted) frame
             * @param[out] decrypted Buffer for decrypted data
             * @param[out] decryptedDataLength Size of decrypted data
             * @return Operation status
             */
            virtual DecryptStatus Decrypt(
                gsl::span<const uint8_t> frame, gsl::span<uint8_t> decrypted, std::size_t& decryptedDataLength) = 0;
        };

        /**
         * Interface describing how incoming frame should be decoded into telecommand
         */
        struct IDecodeTelecommand
        {
            /**
             * @brief Decoded frame into command code and parameters
             * @param[in] frame Incoming frame
             * @param[out] commandCode Extracted command code
             * @param[out] parameters Part of \p frame which contains telecommand parameters
             * @return Operation status
             */
            virtual DecodeFrameStatus Decode(
                gsl::span<const std::uint8_t> frame, std::uint8_t& commandCode, gsl::span<const std::uint8_t>& parameters) = 0;
        };

        /**
         * @brief Interface for classes responsible for handling specific telecommands
         */
        struct IHandleTeleCommand
        {
            /**
             * @brief Method called when telecommand is received.
             * @param[in] parameters Parameters contained in telecommand frame
             */
            virtual void Handle(gsl::span<const std::uint8_t> parameters) = 0;

            /**
             * @brief Returns command code associated with this telecommand
             * @return Command code
             */
            virtual std::uint8_t CommandCode() const = 0;
        };

        /**
         * Incoming frame handler that is capable of decoding them and dispatching telecommands
         */
        class IncomingTelecommandHandler : public devices::comm::IHandleFrame
        {
          public:
            /**
             * Constructs \ref IncomingTelecommandHandler object
             * @param[in] decryptFrame Frame decryption implementation
             * @param[in] decodeTelecommand Telecommand decoding implementation
             * @param[in] telecommands Array of pointers to telecommands
             */
            IncomingTelecommandHandler(
                IDecryptFrame& decryptFrame, IDecodeTelecommand& decodeTelecommand, gsl::span<IHandleTeleCommand*> telecommands);

            /**
             * Handles incoming frame and dispatches (if possible) telecommand
             * @param[in] frame Incoming frame
             */
            virtual void HandleFrame(devices::comm::CommFrame& frame) override;

          private:
            /** @brief Frame decryption implementation */
            IDecryptFrame& _decryptFrame;
            /** @brief Telecommand decoding implementation */
            IDecodeTelecommand& _decodeTelecommand;
            /** @brief Array of pointers to telecommands */
            gsl::span<IHandleTeleCommand*> _telecommands;

            /**
             * Dispatches telecommand handler
             * @param[in] commandCode Command code
             * @param[in] parameters Parameters buffer
             */
            void DispatchCommandHandler(std::uint8_t commandCode, gsl::span<const uint8_t> parameters);
        };
    }
}
/** @} */

#endif /* LIBS_TELECOMMAND_HANDLING_INCLUDE_TELECOMMAND_HANDLING_TELECOMMAND_HANDLING_H_ */
