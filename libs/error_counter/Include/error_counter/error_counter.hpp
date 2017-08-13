#ifndef LIBS_ERROR_COUNTER_INCLUDE_ERROR_COUNTER_ERROR_COUNTER_HPP_
#define LIBS_ERROR_COUNTER_INCLUDE_ERROR_COUNTER_ERROR_COUNTER_HPP_

#include <array>
#include <atomic>
#include <cstdint>
#include <type_traits>
#include "traits.hpp"
#include "utils.h"

namespace error_counter
{
    /**
     * @defgroup error_counter Error counter
     * @ingroup fdir
     *
     * Error counting is based on separate error counters associated with specific element (like device).
     * After successful interactive with device, @ref ErrorCounter::Success() method should be called which will lower current error counter
     * value
     * In case of failure call @ref ErrorCounter::Failure() which will increase counter value. After reaching threshold callback method is
     * called.
     * Values of increment, decrement and threshold can be configured.
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

        /**
         * @brief Returns values by which error counter will be decremented
         * @param device Device
         * @return Decrement value
         */
        virtual CounterValue Decrement(Device device) = 0;
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
     * @brief Interface for error counting mechanism
     */
    struct IErrorCounting
    {
        /**
         * @brief Returns current value of device's error counter
         * @param device Device ID
         * @return Value of error counter
         */
        virtual CounterValue Current(Device device) const = 0;

        /**
         * @brief Records single failure of device
         * @param device Device ID
         */
        virtual void Failure(Device device) = 0;

        /**
         * @brief Records single success of device
         * @param device Device ID
         */
        virtual void Success(Device device) = 0;
    };

    /**
     * @brief Error counting mechanism
     */
    class ErrorCounting final : public IErrorCounting
    {
      public:
        /** @brief Maximum number of supported devices */
        static constexpr std::uint8_t MaxDevices = 12;

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
         * @brief Returns current value of device's error counter
         * @param device Device ID
         * @return Value of error counter
         */
        virtual CounterValue Current(Device device) const override;

        /**
         * @brief Records single failure of device
         * @param device Device ID
         */
        virtual void Failure(Device device) override;

        /**
         * @brief Records single success of device
         * @param device Device ID
         */
        virtual void Success(Device device) override;

      private:
        /** @brief Type that is being used internally to store error counter */
        using InternalCounterValue = std::make_signed_t<Wider<CounterValue>::type>;

        /** @brief Error counters */
        std::array<std::atomic<InternalCounterValue>, MaxDevices> _counters;

        /** @brief Configuration */
        IErrorCountingConfigration& _config;

        /** @brief Callback */
        IErrorCountingCallback* _callback;
    };

    /**
     * @brief Helper class for accessing error counter of single device
     * @tparam Device Device ID
     */
    template <Device Device> class ErrorCounter final
    {
      public:
        /** @brief Error counting mechanism */
        ErrorCounter(IErrorCounting& counting);

        /** @brief Current counter value */
        inline CounterValue Current() const;
        /** @brief Records single failure */
        inline void Failure();
        /** @brief Records single success */
        inline void Success();

        /** @brief Device Id */
        static constexpr auto DeviceId = Device;

      private:
        /** @brief Error counting */
        IErrorCounting& _counting;
    };

    /**
     * @brief Error counter that will report single error even if multiple errors occured.
     */
    class AggregatedErrorCounter
    {
      public:
        /** @brief Ctor */
        AggregatedErrorCounter();

        /** @brief Records single failure */
        inline void Failure();
        /**
         * @brief Current value
         * @return Current aggregated result value
         */
        inline bool GetAggregatedResult() const;

        /**
         * @brief Report the aggregated state to real actual counter
         * @param[in] errorCounter Error counter to report result to
         * @return Aggregated result value
         */
        template <error_counter::Device Device> bool ReportResult(ErrorCounter<Device>& errorCounter);

      private:
        uint32_t _errorCount;
    };

    /**
     * @brief Create instance of this class to guarantee error counter updating in current scope
     */
    template <Device Device> class AggregatedErrorReporter : public NotMoveable, NotCopyable
    {
      public:
        /** @brief Ctor */
        AggregatedErrorReporter(ErrorCounter<Device>& errorCounter);

        /**
         * @brief Destructor reports Failure or Success to error counter
         */
        ~AggregatedErrorReporter();

        /**
         * @brief Gives access to actual error counter
         * @return Reference to error counter used to report errors
         */
        inline AggregatedErrorCounter& Counter();

      private:
        AggregatedErrorCounter _counter;
        ErrorCounter<Device>& _errorCounter;
    };

    template <Device Device>
    AggregatedErrorReporter<Device>::AggregatedErrorReporter(ErrorCounter<Device>& errorCounter) : _errorCounter(errorCounter)
    {
    }

    template <Device Device> AggregatedErrorReporter<Device>::~AggregatedErrorReporter()
    {
        _counter.ReportResult(_errorCounter);
    }

    template <Device Device> AggregatedErrorCounter& AggregatedErrorReporter<Device>::Counter()
    {
        return _counter;
    }

    template <Device Device> ErrorCounter<Device>::ErrorCounter(IErrorCounting& counting) : _counting(counting)
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

    void AggregatedErrorCounter::Failure()
    {
        _errorCount++;
    }

    bool AggregatedErrorCounter::GetAggregatedResult() const
    {
        return _errorCount == 0;
    }

    template <error_counter::Device Device> bool AggregatedErrorCounter::ReportResult(ErrorCounter<Device>& errorCounter)
    {
        return GetAggregatedResult() >> errorCounter;
    }

    /**
     * @brief Operator that can be used to easily kick error counter depending on operation result
     * @param flag Flag determining whether operation was successful
     * @param counter Error counter
     * @return Flag passed as input
     */
    template <Device Device> inline bool operator>>(bool flag, ErrorCounter<Device>& counter)
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

    /**
     * @brief Operator that can be used to easily kick error counter depending on operation result
     * @param flag Flag determining whether operation was successful
     * @param counter Error counter
     * @return Flag passed as input
     */
    inline bool operator>>(bool flag, AggregatedErrorCounter& counter)
    {
        if (!flag)
        {
            counter.Failure();
        }

        return flag;
    }

    /**
     * @brief Interface for object providing error counting telemetry
     */
    struct IErrorCountingTelemetryProvider
    {
        /**
         * @brief Returns reference to error counting mechanism
         * @return Reference to error counting mechanism
         */
        virtual error_counter::ErrorCounting& ErrorCounting() = 0;
    };

    /** @} */
}

#endif /* LIBS_ERROR_COUNTER_INCLUDE_ERROR_COUNTER_ERROR_COUNTER_HPP_ */
