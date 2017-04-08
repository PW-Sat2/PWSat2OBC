#ifndef LIBS_STATE_ANTENNA_ANTENNA_STATE_HPP
#define LIBS_STATE_ANTENNA_ANTENNA_STATE_HPP

#pragma once

namespace state
{
    /**
     * @brief Current antenna deployment state.
     * @ingroup StateDef
     */
    class AntennaState
    {
      public:
        /**
         * @brief .ctor
         */
        AntennaState();

        /**
         * @brief Indicates whether the antenna deployment process is completed.
         * @return Current antenna deployment process status.
         */
        bool IsDeployed() const;

        /**
         * @brief Updates antenna deployment status.
         * @param[in] newState New antenna deployment status.
         */
        void SetDeployment(bool newState);

      private:
        /** @brief Flag indicating that antenna deployment process has been completed. */
        bool deployed;
    };

    inline bool AntennaState::IsDeployed() const
    {
        return this->deployed;
    }

    inline void AntennaState::SetDeployment(bool newState)
    {
        this->deployed = newState;
    }
}

#endif
