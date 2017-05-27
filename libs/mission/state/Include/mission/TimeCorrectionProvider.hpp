#ifndef LIBS_STATE_TIME_INCLUDE_STATE_TIMECORRECTIONPROVIDER_HPP_
#define LIBS_STATE_TIME_INCLUDE_STATE_TIMECORRECTIONPROVIDER_HPP_

#pragma once

#include "state/struct.h"
#include "state/time/ITimeCorrectionProvider.hpp"
#include "utils.h"

namespace mission
{
    /**
     * @brief Time Correction Provider.
     *
     * This type represents proxy operating on Time Correction factors.
     * It can get and set both correction factors (mission time and external mission time)
     * without exposing internal persistent storage to other system parts.
     */
    class TimeCorrectionProvider : public state::ITimeCorrectionProvider
    {
      public:
        /**
         *  @brief Constructor for timer object.
         */
        TimeCorrectionProvider(SystemState& systemState);

        /**
         * @brief Returns current mission time correction factor.
         * @return Mission time correction factor.
         */
        virtual int16_t GetCurrentTimeCorrectionFactor();

        /**
         * @brief Returns current External mission time correction factor.
         * @return External mission time correction factor.
         */
        virtual int16_t GetCurrentExternalTimeCorrectionFactor();

        /**
         * @brief Sets current mission time correction factor.
         *
         * @param[in] factor The new correction factor.
         *
         * @return Operation status. True on success, false otherwise.
         */
        virtual bool SetCurrentTimeCorrectionFactor(int16_t factor);

        /**
         * @brief Sets current external mission time correction factor.
         *
         * @param[in] factor The new correction factor.
         *
         * @return Operation status. True on success, false otherwise.
         */
        virtual bool SetCurrentExternalTimeCorrectionFactor(int16_t factor);

      private:
        SystemState& _systemState;
    };

    /** @}*/
}

#endif /* LIBS_STATE_TIME_INCLUDE_STATE_TIMECORRECTIONPROVIDER_HPP_ */
