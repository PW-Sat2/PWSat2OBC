#ifndef LIBS_OBC_FDIR_INCLUDE_OBC_FDIR_HPP_
#define LIBS_OBC_FDIR_INCLUDE_OBC_FDIR_HPP_

#include <array>
#include <atomic>
#include "error_counter/error_counter.hpp"
#include "power/power.h"

namespace obc
{
    /**
     * @defgroup obc_fdir FDIR
     * @ingroup obc
     *
     * @{
     */

    struct ISetErrorCounterConfig
    {
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
                       public ISetErrorCounterConfig
    {
      public:
        /**
         * @brief Ctor
         * @param powerControl Power control interface
         */
        FDIR(services::power::IPowerControl& powerControl);

        /** @brief Performs initialization */
        void Initalize();

        /**
         * @brief Returns reference to error counting mechanism
         * @return Reference to errour counting mechanism
         */
        error_counter::ErrorCounting& ErrorCounting();

        virtual void Set(error_counter::Device device,
            error_counter::CounterValue limit,
            error_counter::CounterValue increment,
            error_counter::CounterValue decrement) override;

        virtual void LimitReached(error_counter::Device device, error_counter::CounterValue errorsCount) override;
        virtual error_counter::CounterValue Limit(error_counter::Device device) override;
        virtual error_counter::CounterValue Increment(error_counter::Device device) override;
        virtual error_counter::CounterValue Decrement(error_counter::Device device) override;

      private:
        /** @brief Error counting mechanism */
        error_counter::ErrorCounting _errorCounting;

        services::power::IPowerControl& _powerControl;

        std::array<std::atomic<std::uint32_t>, error_counter::ErrorCounting::MaxDevices> _configuration;
    };

    /**@} */
}

#endif /* LIBS_OBC_FDIR_INCLUDE_OBC_FDIR_HPP_ */
