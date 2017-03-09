#ifndef LIBS_TIME_INCLUDE_TIME_ICURRENTTIME_HPP_
#define LIBS_TIME_INCLUDE_TIME_ICURRENTTIME_HPP_

#include <chrono>
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
        };
    }
}

#endif /* LIBS_TIME_INCLUDE_TIME_ICURRENTTIME_HPP_ */
