#ifndef LIBS_STATE_ANTENNA_ANTENNA_STATE_HPP
#define LIBS_STATE_ANTENNA_ANTENNA_STATE_HPP

#pragma once

namespace state
{
    /**
     * @brief Current antenna deployment state.
     */
    class AntennaState
    {
      public:
        AntennaState();

        bool IsDeployed() const;

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
