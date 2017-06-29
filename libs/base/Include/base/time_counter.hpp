#ifndef LIBS_BASE_INCLUDE_BASE_TIME_COUNTER_HPP_
#define LIBS_BASE_INCLUDE_BASE_TIME_COUNTER_HPP_

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>

namespace time_counter
{
    /**
     * @defgroup time_counter Simple time counter
     *
     * This module provides simple downcounter based on time.
     *
     * @{
     */

    /**
     * @brief std::chrono_duration wrapper that can be passed as template parameter
     * @tparam Count number of ticks of given type
     * @tparam Type Duration type
     */
    template <std::int32_t Count, typename Type> struct Duration
    {
        /** @brief Duration value */
        static constexpr Type value = Type(Count);
    };

    /** @brief Helper: miliseconds wrapper */
    template <std::int32_t Count> using ms = Duration<Count, std::chrono::milliseconds>;
    /** @brief Helper: seconds wrapper */
    template <std::int32_t Count> using s = Duration<Count, std::chrono::seconds>;
    /** @brief Helper: minutes wrapper */
    template <std::int32_t Count> using min = Duration<Count, std::chrono::minutes>;

    /**
     * @brief Time counter
     * @tparam Param Type of parameter passed to callback
     * @tparam Period Counter period
     * @tparam StartDelay Initial value of counter. By default 0
     */
    template <typename Action, typename Param, typename Period, typename StartDelay = Duration<0, std::chrono::milliseconds>>
    class TimeCounter
    {
      public:
        /**
         * @brief Ctor
         */
        TimeCounter();

        /**
         * @brief Ctor
         * @param action Callback
         * @param param Parameter that will be passed to callback
         */
        TimeCounter(Action action, Param param);

        /**
         * @brief Returns remaining time
         * @return
         */
        std::chrono::milliseconds TimeToZero();

        /**
         * @brief Performs single step
         * @param delta Amount of time to step by
         * @return Remaining time
         *
         * @remark If specified step will cause stepping over zero, timer will be set to zero
         */
        std::chrono::milliseconds Step(std::chrono::milliseconds delta);

        /**
         * @brief Invokes callback if counter is at zero
         * @return true if callback was invoked, false otherwise
         */
        bool DoOnBottom();

      private:
        /** @brief Current value */
        std::chrono::milliseconds _value;
        /** @brief Callback */
        Action _action;
        /** @brief Parameter for callback */
        Param _param;
    };

    template <typename Action, typename Param, typename Period, typename StartDelay>
    TimeCounter<Action, Param, Period, StartDelay>::TimeCounter() : _value(StartDelay::value), _action(nullptr)
    {
    }

    template <typename Action, typename Param, typename Period, typename StartDelay>
    TimeCounter<Action, Param, Period, StartDelay>::TimeCounter(Action action, Param param)
        : _value(StartDelay::value), _action(action), _param(param)
    {
    }

    template <typename Action, typename Param, typename Period, typename StartDelay>
    std::chrono::milliseconds TimeCounter<Action, Param, Period, StartDelay>::TimeToZero()
    {
        return this->_value;
    }

    template <typename Action, typename Param, typename Period, typename StartDelay>
    std::chrono::milliseconds TimeCounter<Action, Param, Period, StartDelay>::Step(std::chrono::milliseconds delta)
    {
        if (this->_value == decltype(this->_value)::zero())
        {
            this->_value = Period::value;
            this->Step(delta);
        }
        else if (this->_value < delta)
        {
            this->_value = decltype(this->_value)::zero();
        }
        else
        {
            this->_value -= delta;
        }

        return this->_value;
    }

    template <typename Action, typename Param, typename Period, typename StartDelay>
    bool TimeCounter<Action, Param, Period, StartDelay>::DoOnBottom()
    {
        if (this->_value == decltype(this->_value)::zero())
        {
            if (this->_action != nullptr)
            {
                this->_action(this->_param);
            }
            this->_value = Period::value;
            return true;
        }
        return false;
    }

    /**
     * @brief Finds minimal time to move at least one counter to zero
     * @param counters Counters
     * @return Duration to move at least one counter to zero
     */
    template <typename... Counters> std::chrono::milliseconds SleepTime(Counters&... counters)
    {
        static_assert(sizeof...(Counters) > 0, "Specify at least one counter");

        std::array<std::chrono::milliseconds, sizeof...(Counters)> times{counters.TimeToZero()...};
        auto m = std::min_element(times.begin(), times.end());

        return *m;
    }

    /**
     * @brief Performs step on all counters
     * @param delta Time to step by
     * @param counters Counters
     */
    template <typename... Counters> void Step(std::chrono::milliseconds delta, Counters&... counters)
    {
        static_assert(sizeof...(Counters) > 0, "Specify at least one counter");
        std::array<std::chrono::milliseconds, sizeof...(Counters)>{counters.Step(delta)...};
    }

    /**
     * @brief Invokes callback on all counters
     * @param counters Counters
     */
    template <typename... Counters> void DoOnBottom(Counters&... counters)
    {
        static_assert(sizeof...(Counters) > 0, "Specify at least one counter");
        std::array<bool, sizeof...(Counters)>{counters.DoOnBottom()...};
    }

    /** @} */
}

#endif /* LIBS_BASE_INCLUDE_BASE_TIME_COUNTER_HPP_ */
