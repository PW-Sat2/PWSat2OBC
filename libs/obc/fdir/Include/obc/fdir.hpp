#ifndef LIBS_OBC_FDIR_INCLUDE_OBC_FDIR_HPP_
#define LIBS_OBC_FDIR_INCLUDE_OBC_FDIR_HPP_

#include <array>
#include <atomic>
#include "error_counter/error_counter.hpp"
#include "mission/fdir.hpp"
#include "power/power.h"

namespace obc
{
    /**
     * @defgroup obc_fdir FDIR
     * @ingroup obc
     *
     * @{
     */

    /**
     * @brief Interface for object responsible for setting error counting configuration
     */
    struct ISetErrorCounterConfig
    {
        /**
         * @brief Sets error counter config for single device
         * @param device Device ID
         * @param limit Error limit
         * @param increment Increment value on error
         * @param decrement Decrement value on success
         */
        virtual void Set(error_counter::Device device,
            error_counter::CounterValue limit,
            error_counter::CounterValue increment,
            error_counter::CounterValue decrement) = 0;
    };

    /**
     * @brief FDIR mechanims for OBC
     */
    class FDIR final : private error_counter::IErrorCountingCallback,
                       public error_counter::IErrorCountingConfigration,
                       public ISetErrorCounterConfig,
                       public mission::IGetErrorCounterConfig,
                       public error_counter::IErrorCountingTelemetryProvider
    {
      public:
        /**
         * @brief Ctor
         * @param powerControl Power control interface
         * @param maskedDevices Bitset for devices that will ignored when reaching error limit
         */
        FDIR(services::power::IPowerControl& powerControl, std::uint16_t maskedDevices);

        /** @brief Performs initialization */
        void Initalize();

        /**
         * @brief Loads config from packed form
         * @param config Packed form of error counters config
         */
        void LoadConfig(std::array<std::uint32_t, error_counter::ErrorCounting::MaxDevices>& config);

        /**
         * @brief Returns reference to error counting mechanism
         * @return Reference to errour counting mechanism
         */
        virtual error_counter::ErrorCounting& ErrorCounting() override;

        virtual void Set(error_counter::Device device,
            error_counter::CounterValue limit,
            error_counter::CounterValue increment,
            error_counter::CounterValue decrement) override;

        virtual void LimitReached(error_counter::Device device, error_counter::CounterValue errorsCount) override;
        virtual error_counter::CounterValue Limit(error_counter::Device device) override;
        virtual error_counter::CounterValue Increment(error_counter::Device device) override;
        virtual error_counter::CounterValue Decrement(error_counter::Device device) override;

        virtual std::array<std::uint32_t, error_counter::ErrorCounting::MaxDevices> GetConfig() override;

      private:
        /** @brief Error counting mechanism */
        error_counter::ErrorCounting _errorCounting;

        /** @brief Power control */
        services::power::IPowerControl& _powerControl;

        /** @brief Error counting configuration */
        std::array<std::atomic<std::uint32_t>, error_counter::ErrorCounting::MaxDevices> _configuration;

        /** @brief Masked devices */
        std::uint16_t _maskedDevices;
    };

    /**@} */
}

#endif /* LIBS_OBC_FDIR_INCLUDE_OBC_FDIR_HPP_ */
