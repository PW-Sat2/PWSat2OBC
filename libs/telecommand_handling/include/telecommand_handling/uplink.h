#ifndef LIBS_TELECOMMAND_HANDLING_INCLUDE_TELECOMMAND_HANDLING_UPLINK_H_
#define LIBS_TELECOMMAND_HANDLING_INCLUDE_TELECOMMAND_HANDLING_UPLINK_H_

#include <cstdint>
#include <gsl/span>

#include "telecommand_handling.h"

namespace telecommands
{
    /**
     * @brief Uplink telecommand protocol handler
     *
     * This is just dummy implementation that assumes no encoding/encryption and uses first byte in frame as command code.
     */
    class UplinkProtocol final : public telecommands::handling::IDecryptFrame, public telecommands::handling::IDecodeTelecommand
    {
      public:
        /**
         * @brief Copies input frame to @p decrypted buffer
         * @param[in] frame Input frame
         * @param[in] decrypted Buffer for decrypted data
         * @return Always success
         */
        virtual telecommands::handling::DecryptFrameResult Decrypt(
            gsl::span<const std::uint8_t> frame, gsl::span<std::uint8_t> decrypted) override;

        /**
         * @brief Decodes frame into command code (first byte) and parameters (rest of frame)
         * @param[in] frame Incoming frame
         * @return Decoding result
         */
        virtual telecommands::handling::DecodeTelecommandResult Decode(gsl::span<const std::uint8_t> frame) override;
    };
}

#endif /* LIBS_TELECOMMAND_HANDLING_INCLUDE_TELECOMMAND_HANDLING_UPLINK_H_ */
