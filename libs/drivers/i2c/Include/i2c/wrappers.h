#ifndef LIBS_DRIVERS_I2C_INCLUDE_I2C_WRAPPERS_H_
#define LIBS_DRIVERS_I2C_INCLUDE_I2C_WRAPPERS_H_

#include "i2c.h"

namespace drivers
{
    namespace i2c
    {
        /**
         * @ingroup i2c
         * @{
         */

        /**
         * @brief I2C Fallbacking bus driver*
         */
        class I2CFallbackBus final : public II2CBus
        {
          public:
            /**
             * @brief Setups bus wrapper that fallbacks from system to payload bus in case of failure
             * @param[in] buses Object representing both buses used in the system
             */
            I2CFallbackBus(I2CInterface& buses);

            virtual I2CResult Write(const I2CAddress address, gsl::span<const uint8_t> inData) override;

            virtual I2CResult WriteRead(const I2CAddress address, gsl::span<const uint8_t> inData, gsl::span<uint8_t> outData) override;

          private:
            /** @brief Object representing both buses used in the system */
            I2CInterface& _innerBuses;
        };

        /**
         * @brief Type of procedure used by error handling bus wrapper
         * @param[in] bus Bus on which transfer failed
         * @param[in] result Transfer error code
         * @param[in] address Device that was addressed
         * @param[in] context Context
         * @return New result code
         */
        using BusErrorHandler = I2CResult (*)(II2CBus& bus, I2CResult result, I2CAddress address, void* context);

        /**
         * @brief Error handling bus driver
         */
        class I2CErrorHandlingBus final : public II2CBus
        {
          public:
            /**
             * @brief Initializes new instance of @ref I2CErrorHandlingBus
             * @param[in] innerBus Bus that will be wrapped
             * @param[in] handler Pointer to error handler function
             * @param[in] context Parameter passed to error handler function
             */
            I2CErrorHandlingBus(II2CBus& innerBus, BusErrorHandler handler, void* context);

            virtual I2CResult Write(const I2CAddress address, gsl::span<const uint8_t> inData) override;

            virtual I2CResult WriteRead(const I2CAddress address, gsl::span<const uint8_t> inData, gsl::span<uint8_t> outData) override;

          private:
            /** @brief Underlying bus */
            II2CBus& _innerBus;
            /** @brief Pointer to function called in case of error */
            const BusErrorHandler _errorHandler;
            /** @brief Context passed to error handler function */
            void* _handlerContext;
        };

        /** @} */
    }
}

#endif /* LIBS_DRIVERS_I2C_INCLUDE_I2C_WRAPPERS_H_ */
