#ifndef LIBS_TELECOMMUNICATION_INCLUDE_TELECOMMUNICATION_UPLINK_H_
#define LIBS_TELECOMMUNICATION_INCLUDE_TELECOMMUNICATION_UPLINK_H_

#include <cstdint>
#include <gsl/span>
#include "telecommand_handling.h"

namespace telecommunication
{
    namespace uplink
    {
        /**
         * @brief Uplink telecommand protocol handler
         * @ingroup telecomm_handling
         */
        class UplinkProtocol final : public IDecodeTelecommand
        {
          public:
            /**
             * @brief Initializes @ref UplinkProtocol instance
             * @param[in] securityCode Security code
             */
            UplinkProtocol(std::uint32_t securityCode);

            /**
             * @brief Decodes frame into command code (first byte) and parameters (rest of frame)
             * @param[in] frame Incoming frame
             * @return Decoding result
             */
            virtual DecodeTelecommandResult Decode(gsl::span<const std::uint8_t> frame) override;

          private:
            /** @brief Security code */
            const std::uint32_t _securityCode;
        };
    }
}

#endif /* LIBS_TELECOMMUNICATION_INCLUDE_TELECOMMUNICATION_UPLINK_H_ */
