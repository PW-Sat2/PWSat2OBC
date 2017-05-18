#ifndef LIBS_STATE_LOCKABLE_PERSISTENT_STATE_HPP
#define LIBS_STATE_LOCKABLE_PERSISTENT_STATE_HPP

#include "PersistentState.hpp"
#include "logger/logger.h"

namespace state
{
    template <typename StatePolicy, typename... Parts> class LockablePersistentState : public PersistentState<StatePolicy, Parts...>
    {
      public:
        using Base = PersistentState<StatePolicy, Parts...>;

        LockablePersistentState();

        OSResult Initialize();

        /**
         * @brief Return reference to selected part of the persistent state.
         * @tparam Object Type of the object that should be accessed.
         * @return Reference to object of the requested type.
         */
        template <typename Object> const Object& Get() const;

        /**
         * @brief Updates value of the selected part of the persistent state.
         *
         * Setting any part of the state will mark the persistent state object as modified.
         * @param[in] object New value of the selected part of the persistent state.
         * @tparam Object Type of the object that should be accessed.
         */
        template <typename Object> void Set(const Object& object);

        /**
         * @brief Read the persistent state from the passed object reader.
         * @param[in] reader Reader object that should be used to access the serialized state.
         */
        void Read(Reader& reader);

        /**
         * @brief Write the persistent state to the passed object writer.
         * @param[in] writer Writer object that should be used to save the serialized state.
         */
        void Write(Writer& writer) const;

        /**
         * @brief Write the persistent state to the passed object writer.
         *
         * This method will reset modified mark of the object once the saving process is complete.
         * @param[in] writer Writer object that should be used to save the serialized state.
         */
        void Capture(Writer& writer) const;

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

      private:
        OSSemaphoreHandle synchronizationLock;
    };

    template <typename StatePolicy, typename... Parts>
    LockablePersistentState<StatePolicy, Parts...>::LockablePersistentState() : synchronizationLock(System::CreateBinarySemaphore())
    {
    }

    template <typename StatePolicy, typename... Parts> OSResult LockablePersistentState<StatePolicy, Parts...>::Initialize()
    {
        return System::GiveSemaphore(this->synchronizationLock);
    }

    template <typename StatePolicy, typename... Parts>
    template <typename Object>
    const Object& LockablePersistentState<StatePolicy, Parts...>::Get() const
    {
        Lock lock(this->synchronizationLock, InfiniteTimeout);
        if (!lock())
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire PersistentState lock.");
        }

        return Base::template Get<Object>();
    }

    template <typename StatePolicy, typename... Parts>
    template <typename Object>
    void LockablePersistentState<StatePolicy, Parts...>::Set(const Object& object)
    {
        Lock lock(this->synchronizationLock, InfiniteTimeout);
        if (!lock())
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire PersistentState lock.");
        }

        Base::Set(object);
    }

    template <typename StatePolicy, typename... Parts> void LockablePersistentState<StatePolicy, Parts...>::Read(Reader& reader)
    {
        Lock lock(this->synchronizationLock, InfiniteTimeout);
        if (!lock())
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire PersistentState lock.");
        }

        Base::Read(reader);
    }

    template <typename StatePolicy, typename... Parts> void LockablePersistentState<StatePolicy, Parts...>::Write(Writer& writer) const
    {
        Lock lock(this->synchronizationLock, InfiniteTimeout);
        if (!lock())
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire PersistentState lock.");
        }

        Base::Write(writer);
    }

    template <typename StatePolicy, typename... Parts> void LockablePersistentState<StatePolicy, Parts...>::Capture(Writer& writer) const
    {
        Lock lock(this->synchronizationLock, InfiniteTimeout);
        if (!lock())
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire PersistentState lock.");
        }

        Base::Capture(writer);
    }

    template <typename StatePolicy, typename... Parts> bool LockablePersistentState<StatePolicy, Parts...>::IsModified() const
    {
        Lock lock(this->synchronizationLock, InfiniteTimeout);
        if (!lock())
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire PersistentState lock.");
        }

        return Base::IsModified();
    }

    template <typename StatePolicy, typename... Parts> inline constexpr std::uint32_t LockablePersistentState<StatePolicy, Parts...>::Size()
    {
        return Base::Size();
    }
}

#endif
