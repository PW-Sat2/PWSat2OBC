#ifndef LIBS_TELECOMMAND_HANDLING_INCLUDE_TELECOMMAND_HANDLING_TELECOMMAND_HANDLING_H_
#define LIBS_TELECOMMAND_HANDLING_INCLUDE_TELECOMMAND_HANDLING_TELECOMMAND_HANDLING_H_

#include <cstdint>
#include <gsl/span>
#include "comm/IHandleFrame.hpp"

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
         * @brief Reason of frame decryption failure
         */
        enum class DecryptFrameFailureReason
        {
            GeneralError //!< GeneralError
        };

        /**
         * @brief Result of frame decryption.  It is (sort of) discrinated union: Success | Failure
         */
        class DecryptFrameResult
        {
          public:
            /**
             * @brief Crates success result
             * @param[in] decrypted Decrypted frame
             * @return Success result
             */
            static const DecryptFrameResult Success(gsl::span<const std::uint8_t> decrypted);

            /**
             * @brief Creates failure result
             * @param[in] reason Failure reason
             * @return Failure result
             */
            static const DecryptFrameResult Failure(DecryptFrameFailureReason reason);

            /** @brief Differentiates success from failure */
            const bool IsSuccess;
            /** @brief (Success-only) Decrypted frame */
            const gsl::span<const std::uint8_t> Decrypted;
            /** @brief (Failure-only) Failure reason */
            const DecryptFrameFailureReason FailureReason;

          private:
            /**
             * @brief Constructor for success case
             * @param[in] decrypted Decrypted frame
             */
            DecryptFrameResult(gsl::span<const std::uint8_t> decrypted);

            /**
             * @brief Constructor for failure case
             * @param[in] reason Failure reason
             */
            DecryptFrameResult(DecryptFrameFailureReason reason);
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
             * @return Operation status
             */
            virtual DecryptFrameResult Decrypt(gsl::span<const uint8_t> frame, gsl::span<uint8_t> decrypted) = 0;
        };

        /**
         * @brief Reason of telecommand decoding failure
         */
        enum class DecodeTelecommandFailureReason
        {
            GeneralError //!< General error
        };

        /**
         * @brief Result of telecommand decoding. It is (sort of) discrinated union: Success | Failure
         */
        class DecodeTelecommandResult
        {
          public:
            /**
             * @brief Creates success result
             * @param[in] commandCode Command code
             * @param[in] parameters Telecommand parameters
             * @return Success result
             */
            static const DecodeTelecommandResult Success(std::uint8_t commandCode, gsl::span<const std::uint8_t> parameters);

            /**
             * @brief Creates failure result
             * @param[in] reason Failure result
             * @return Failure result
             */
            static const DecodeTelecommandResult Failure(DecodeTelecommandFailureReason reason);

            /** @brief Differentiates success from failure */
            const bool IsSuccess;
            /** @brief (Success-only) Command code */
            const std::uint8_t CommandCode;
            /** @brief (Success-only) Telecommand parameters */
            const gsl::span<const std::uint8_t> Parameters;
            /** @brief (Failure-only) Failure reason */
            const DecodeTelecommandFailureReason FailureReason;

          private:
            /**
             * @brief Constructor for failure case
             * @param[in] commandCode Command code
             * @param[in] parameters Telecommand parameters
             */
            DecodeTelecommandResult(std::uint8_t commandCode, gsl::span<const std::uint8_t> parameters);
            /**
             * @brief Constructor for failure case
             * @param[in] reason Failure reason
             */
            DecodeTelecommandResult(DecodeTelecommandFailureReason reason);
        };

        /**
         * Interface describing how incoming frame should be decoded into telecommand
         */
        struct IDecodeTelecommand
        {
            /**
             * @brief Decoded frame into command code and parameters
             * @param[in] frame Incoming frame
             * @return Operation status
             */
            virtual DecodeTelecommandResult Decode(gsl::span<const std::uint8_t> frame) = 0;
        };

        /**
         * @brief Interface for classes responsible for handling specific telecommands
         */
        struct IHandleTeleCommand
        {
            /**
             * @brief Method called when telecommand is received.
             * @param[in] transmitter Reference to object that can be used to send response back
             * @param[in] parameters Parameters contained in telecommand frame
             */
            virtual void Handle(devices::comm::ITransmitFrame& transmitter, gsl::span<const std::uint8_t> parameters) = 0;

            /**
             * @brief Returns command code associated with this telecommand
             * @return Command code
             */
            virtual std::uint8_t CommandCode() const = 0;
        };

        /**
         * Incoming frame handler that is capable of decoding them and dispatching telecommands
         */
        class IncomingTelecommandHandler final : public devices::comm::IHandleFrame
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
             * @param[in] transmitter Reference to object used to send response back
             * @param[in] frame Incoming frame
             */
            virtual void HandleFrame(devices::comm::ITransmitFrame& transmitter, devices::comm::Frame& frame) override;

          private:
            /**
             * Dispatches telecommand handler
             * @param[in] transmitter Transmitter used to send response back
             * @param[in] commandCode Command code
             * @param[in] parameters Parameters buffer
             */
            void DispatchCommandHandler(
                devices::comm::ITransmitFrame& transmitter, std::uint8_t commandCode, gsl::span<const uint8_t> parameters);

            /** @brief Frame decryption implementation */
            IDecryptFrame& _decryptFrame;
            /** @brief Telecommand decoding implementation */
            IDecodeTelecommand& _decodeTelecommand;
            /** @brief Array of pointers to telecommands */
            gsl::span<IHandleTeleCommand*> _telecommands;
        };
    }
}
/** @} */

#endif /* LIBS_TELECOMMAND_HANDLING_INCLUDE_TELECOMMAND_HANDLING_TELECOMMAND_HANDLING_H_ */
