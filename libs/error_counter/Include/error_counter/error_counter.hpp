#ifndef LIBS_ERROR_COUNTER_INCLUDE_ERROR_COUNTER_ERROR_COUNTER_HPP_
#define LIBS_ERROR_COUNTER_INCLUDE_ERROR_COUNTER_ERROR_COUNTER_HPP_

#include <array>
#include <cstdint>
#include "base/os.h"

namespace error_counter
{
    /**
     * @defgroup error_counter Error counter
     * @ingroup fdir
     *
     * @{
     */

    /** @brief Type used to represent device ID */
    using Device = std::uint8_t;
    /** @brief Type used to represent error counter value */
    using CounterValue = std::uint8_t;

    /**
     * @brief Interface for object providing error counting configuration
     */
    struct IErrorCountingConfigration
    {
        /**
         * @brief Returns error limit for given device
         * @param device Device ID
         * @return Error limit
         */
        virtual CounterValue Limit(Device device) = 0;
        /**
         * @brief Returns value by which error counter will be incremented
         * @param device Device ID
         * @return Increment value
         */
        virtual CounterValue Increment(Device device) = 0;
    };

    /**
     * @brief Interface for object being called by error counting in case of limit reached
     */
    struct IErrorCountingCallback
    {
        /**
         * @brief Callback called when device reaches its error limit
         * @param device Device ID
         * @param errorsCount Errors count
         */
        virtual void LimitReached(Device device, CounterValue errorsCount) = 0;
    };

    /**
     * @brief Error counting mechanism
     *
     * Each device has associated error counter.
     * On success, it is set to zero.
     * On error, it is incremented by value retrieved from config interface
     * When error limit (also configurable) is reach, callback is invoked
     */
    class ErrorCounting
    {
      public:
        /** @brief Maximum number of supported devices */
        static constexpr std::uint8_t MaxDevices = 10;

        /**
         * @brief Ctor
         * @param config Configuration callback
         */
        ErrorCounting(IErrorCountingConfigration& config);

        /**
         * @brief Sets callback
         * @param callback Callback
         *
         * @remark Previous callback will be detached.
         */
        void Handler(IErrorCountingCallback& callback);

        /**
         * @brief Initializes error counting
         */
        void Initialize();

        /**
         * @brief Returns current value of device's error counter
         * @param device Device ID
         * @return Value of error counter
         */
        CounterValue Current(Device device) const;

        /**
         * @brief Records single failure of device
         * @param device Device ID
         */
        void Failure(Device device);

        /**
         * @brief Records single success of device
         * @param device Device ID
         */
        void Success(Device device);

      private:
        /** @brief Error counters */
        std::array<CounterValue, MaxDevices> _counters;
        /** @brief Configuration */
        IErrorCountingConfigration& _config;
        /** @brief Synchronization semaphore */
        OSSemaphoreHandle _sync;
        /** @brief Callback */
        IErrorCountingCallback* _callback;
    };

    /**
     * @brief Helper class for accessing error counter of single device
     * @tparam Device Device ID
     */
    template <Device Device> class ErrorCounter
    {
      public:
        /** @brief Error counting mechanism */
        ErrorCounter(ErrorCounting& counting);

        /** @brief Current counter value */
        CounterValue Current() const;
        /** @brief Records single failure */
        void Failure();
        /** @brief Records single success */
        void Success();

      private:
        /** @brief Error counting */
        ErrorCounting& _counting;
    };

    template <Device Device> ErrorCounter<Device>::ErrorCounter(ErrorCounting& counting) : _counting(counting)
    {
    }

    template <Device Device> std::uint8_t ErrorCounter<Device>::Current() const
    {
        return this->_counting.Current(Device);
    }

    template <Device Device> void ErrorCounter<Device>::Failure()
    {
        this->_counting.Failure(Device);
    }

    template <Device Device> void ErrorCounter<Device>::Success()
    {
        this->_counting.Success(Device);
    }

    /**
     * @brief Operator that can be used to easily kick error counter depending on operation result
     * @param flag Flag determining whether operation was successful
     * @param counter Error counter
     * @return Flag passed as input
     */
    template <Device Device> bool operator>>(bool flag, ErrorCounter<Device>& counter)
    {
        if (flag)
        {
            counter.Success();
        }
        else
        {
            counter.Failure();
        }

        return flag;
    }

    /** @} */
}

#endif /* LIBS_ERROR_COUNTER_INCLUDE_ERROR_COUNTER_ERROR_COUNTER_HPP_ */
