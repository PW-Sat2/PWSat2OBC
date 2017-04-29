#ifndef LIBS_STATE_STATE_POLICIES_HPP
#define LIBS_STATE_STATE_POLICIES_HPP

#pragma once

namespace state
{
    /**
     * @ingroup StateDef
     * @brief State tracking policy for Persistent state.
     *
     * This policy does not track anything but always report that state has been changed.
     */
    struct NoTrackingStatePolicy
    {
        /**
         * @brief Handler for notification that state has been changed.
         */
        void NotifyModified()
        {
        }

        /**
         * @brief Handler for notification that state has been saved.
         */
        void NotifySaved()
        {
        }

        /**
         * @brief Returns information whether state has been changed since last save.
         * @retval true State has been changed changed since last save.
         * @retval false State has not been changed changed since last save.
         */
        bool IsModified() const
        {
            return true;
        }
    };

    /**
     * @ingroup StateDef
     * @brief State tracking policy for Persistent state.
     *
     *
     * This policy keeps track of the notifications from the persistent state implementation.
     */
    class StateTrackingPolicy
    {
      public:
        /**
         * @brief Handler for notification that state has been changed.
         */
        void NotifyModified();

        /**
         * @brief Handler for notification that state has been saved.
         */
        void NotifySaved();

        /**
         * @brief Returns information whether state has been changed since last save.
         * @retval true State has been changed changed since last save.
         * @retval false State has not been changed changed since last save.
         */
        bool IsModified() const;

      private:
        /**
         * @brief Flag indicating whether state has been changed.
         */
        bool isModified = false;
    };

    inline void StateTrackingPolicy::NotifyModified()
    {
        this->isModified = true;
    }

    inline void StateTrackingPolicy::NotifySaved()
    {
        this->isModified = false;
    }

    inline bool StateTrackingPolicy::IsModified() const
    {
        return this->isModified;
    }
}

#endif
