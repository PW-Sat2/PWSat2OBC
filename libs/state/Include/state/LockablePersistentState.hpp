#ifndef LIBS_STATE_LOCKABLE_PERSISTENT_STATE_HPP
#define LIBS_STATE_LOCKABLE_PERSISTENT_STATE_HPP

#include "PersistentState.hpp"
#include "logger/logger.h"
#include "utils.h"

namespace state
{
    /**
     * @brief Thread-safe access to persistent state
     */
    template <typename StatePolicy, typename... Parts> class LockablePersistentState : NotCopyable
    {
      public:
        /**
         * @brief Default ctor
         */
        LockablePersistentState();

        /**
         * @brief Initializes persistent state.
         * @returns Operation status.
         */
        OSResult Initialize();

        /**
         * @brief Gets reference to selected part of the persistent state.
         * @tparam Object Type of the object that should be accessed.
         * @param[out] object Reference to object of the requested type.
         * @return true if object retrieval was successful.
         */
        template <typename Object> bool Get(Object& object) const;

        /**
         * @brief Updates value of the selected part of the persistent state.
         *
         * Setting any part of the state will mark the persistent state object as modified.
         * @param[in] object New value of the selected part of the persistent state.
         * @tparam Object Type of the object that should be accessed.
         * @return true if object was successfully set.
         */
        template <typename Object> bool Set(const Object& object);

        /**
         * @brief Read the persistent state from the passed object reader.
         * @param[in] reader Reader object that should be used to access the serialized state.
         * @return true if read was successful.
         */
        bool Read(Reader& reader);

        /**
         * @brief Write the persistent state to the passed object writer.
         * @param[in] writer Writer object that should be used to save the serialized state.
         * @return true if write was successful.
         */
        bool Write(Writer& writer) const;

        /**
         * @brief Write the persistent state to the passed object writer.
         *
         * This method will reset modified mark of the object once the saving process is complete.
         * @param[in] writer Writer object that should be used to save the serialized state.
         * @return true if capture was successful.
         */
        bool Capture(Writer& writer) const;

        /**
         * @brief Returns information if the persistent state object has been modified since last state save.
         * @return True if there were some state modifications, false otherwise.
         */
        bool IsModified() const;

        /**
         * @brief Returns size of the entire serialized state in bytes.
         * @return Size of the entire serialized state in bytes.
         */
        static constexpr std::uint32_t Size();

        /**
         * @brief Describes the type of internal PersistentState.
         */
        using InternalPersistentState = PersistentState<StatePolicy, Parts...>;

        /**
         * @brief Loads new State into internal persistent state.
         * @param[in] newState Reference to new state
         * @return True if loading was successful.
         */
        bool Load(const InternalPersistentState& newState);

      private:
        /** @brief Persistent state container. */
        InternalPersistentState state;

        /** @brief Semaphore used for task synchronization. */
        OSSemaphoreHandle synchronizationLock;
    };

    template <typename StatePolicy, typename... Parts>
    LockablePersistentState<StatePolicy, Parts...>::LockablePersistentState()
        : state(), synchronizationLock(System::CreateBinarySemaphore())
    {
    }

    template <typename StatePolicy, typename... Parts> OSResult LockablePersistentState<StatePolicy, Parts...>::Initialize()
    {
        return System::GiveSemaphore(this->synchronizationLock);
    }

    template <typename StatePolicy, typename... Parts>
    template <typename Object>
    bool LockablePersistentState<StatePolicy, Parts...>::Get(Object& object) const
    {
        Lock lock(this->synchronizationLock, InfiniteTimeout);
        if (!lock())
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire PersistentState lock.");
            return false;
        }

        object = state.Get<Object>();

        return true;
    }

    template <typename StatePolicy, typename... Parts>
    template <typename Object>
    bool LockablePersistentState<StatePolicy, Parts...>::Set(const Object& object)
    {
        Lock lock(this->synchronizationLock, InfiniteTimeout);
        if (!lock())
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire PersistentState lock.");
            return false;
        }

        state.Set(object);

        return true;
    }

    template <typename StatePolicy, typename... Parts> bool LockablePersistentState<StatePolicy, Parts...>::Read(Reader& reader)
    {
        Lock lock(this->synchronizationLock, InfiniteTimeout);
        if (!lock())
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire PersistentState lock.");
            return false;
        }

        state.Read(reader);

        return true;
    }

    template <typename StatePolicy, typename... Parts> bool LockablePersistentState<StatePolicy, Parts...>::Write(Writer& writer) const
    {
        Lock lock(this->synchronizationLock, InfiniteTimeout);
        if (!lock())
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire PersistentState lock.");
            return false;
        }

        state.Write(writer);

        return true;
    }

    template <typename StatePolicy, typename... Parts> bool LockablePersistentState<StatePolicy, Parts...>::Capture(Writer& writer) const
    {
        Lock lock(this->synchronizationLock, InfiniteTimeout);
        if (!lock())
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire PersistentState lock.");
            return false;
        }

        state.Capture(writer);

        return true;
    }

    template <typename StatePolicy, typename... Parts> bool LockablePersistentState<StatePolicy, Parts...>::IsModified() const
    {
        Lock lock(this->synchronizationLock, InfiniteTimeout);
        if (!lock())
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire PersistentState lock.");
            return false;
        }

        return state.IsModified();
    }

    template <typename StatePolicy, typename... Parts> inline constexpr std::uint32_t LockablePersistentState<StatePolicy, Parts...>::Size()
    {
        return PersistentState<StatePolicy, Parts...>::Size();
    }

    template <typename StatePolicy, typename... Parts>
    bool LockablePersistentState<StatePolicy, Parts...>::Load(const InternalPersistentState& newState)
    {
        Lock lock(this->synchronizationLock, InfiniteTimeout);
        if (!lock())
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire PersistentState lock.");
            return false;
        }

        state = newState;

        return true;
    }
}

#endif
