#ifndef LIBS_MISSION_TIME_ITIMECORRECTIONPROVIDER_HPP_
#define LIBS_MISSION_TIME_ITIMECORRECTIONPROVIDER_HPP_

#include "utils.h"

namespace state
{
    /**
     * @brief Current time provider
     * @ingroup time
     */
    struct ITimeCorrectionProvider
    {
        /**
         * @brief Returns current mission time correction factor.
         * @return Mission time correction factor.
         */
        virtual int16_t GetCurrentTimeCorrectionFactor() = 0;

        /**
         * @brief Returns current External mission time correction factor.
         * @return External mission time correction factor.
         */
        virtual int16_t GetCurrentExternalTimeCorrectionFactor() = 0;

        /**
         * @brief Sets current mission time correction factor.
         *
         * @param[in] factor The new correction factor.
         *
         * @return Operation status. True on success, false otherwise.
         */
        virtual bool SetCurrentTimeCorrectionFactor(int16_t factor) = 0;

        /**
         * @brief Sets current external mission time correction factor.
         *
         * @param[in] factor The new correction factor.
         *
         * @return Operation status. True on success, false otherwise.
         */
        virtual bool SetCurrentExternalTimeCorrectionFactor(int16_t factor) = 0;
    };
}

#endif /* LIBS_MISSION_TIME_ITIMECORRECTIONPROVIDER_HPP_ */
