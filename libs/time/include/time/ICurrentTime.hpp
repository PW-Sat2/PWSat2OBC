#ifndef LIBS_TIME_INCLUDE_TIME_ICURRENTTIME_HPP_
#define LIBS_TIME_INCLUDE_TIME_ICURRENTTIME_HPP_

#include <chrono>
#include "TimePoint.h"
#include "utils.h"

namespace services
{
    namespace time
    {
        /**
         * @brief Current time provider
         * @ingroup time
         */
        struct ICurrentTime
        {
            /**
             * @brief Returns current mission time
             * @return Mission time
             */
            virtual Option<std::chrono::milliseconds> GetCurrentTime() = 0;

            /**
             * @brief This procedure sets the current mission time to any arbitrary point in time.
             *
             * The currently saved time gets immediately preserved and propagated to the notification routine.
             * @param[in] pointInTime New timer state.
             *
             * @return Operation status. True on success, false otherwise.
             */
            virtual bool SetCurrentTime(TimePoint pointInTime) = 0;

            /**
             * @brief This procedure sets the current mission time to any arbitrary point in time.
             *
             * The currently saved time gets immediately preserved and propagated to the notification routine.
             * @param[in] duration New timer state.
             *
             * @return Operation status. True on success, false otherwise.
             */
            virtual bool SetCurrentTime(std::chrono::milliseconds duration) = 0;
        };
    }
}

#endif /* LIBS_TIME_INCLUDE_TIME_ICURRENTTIME_HPP_ */
