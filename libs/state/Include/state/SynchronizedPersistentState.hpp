#ifndef LIBS_STATE_SYNCHRONIZED_PERSISTENT_STATE_HPP
#define LIBS_STATE_SYNCHRONIZED_PERSISTENT_STATE_HPP

#include "state/PersistentState.hpp"

namespace state
{
    template <typename StatePolicy, typename... Parts> class SynchronizedPersistentState
    {
      public:
        SynchronizedPersistentState(PersistentState<StatePolicy, Parts>& state);

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
        PersistentState<StatePolicy, Parts>& state;

        /** @brief Lock object for synchronization */
        OSSemaphoreHandle lock;
    };

    template <typename StatePolicy, typename... Parts> OSResult SynchronizedPersistentState<StatePolicy, Parts...>::Initialize()
    {
        return System::GiveSemaphore(this->lock);
    }

    template <typename StatePolicy, typename... Parts>
    template <typename Object>
    const Object& SynchronizedPersistentState<StatePolicy, Parts...>::Get() const
    {
        return this->state.Get();
    }

    template <typename StatePolicy, typename... Parts>
    template <typename Object>
    void SynchronizedPersistentState<StatePolicy, Parts...>::Set(const Object& object)
    {
        this->state.Set(object);
    }

    template <typename StatePolicy, typename... Parts> void SynchronizedPersistentState<StatePolicy, Parts...>::Read(Reader& reader)
    {
        this->state.Read(reader);
    }

    template <typename StatePolicy, typename... Parts> void SynchronizedPersistentState<StatePolicy, Parts...>::Write(Writer& writer) const
    {
        this->state.Write(writer);
    }

    template <typename StatePolicy, typename... Parts>
    void SynchronizedPersistentState<StatePolicy, Parts...>::Capture(Writer& writer) const
    {
        this->state.Capture(writer);
    }

    template <typename StatePolicy, typename... Parts> bool SynchronizedPersistentState<StatePolicy, Parts...>::IsModified() const
    {
        return this->state.IsModified();
    }

    template <typename StatePolicy, typename... Parts>
    inline constexpr std::uint32_t SynchronizedPersistentState<StatePolicy, Parts...>::Size()
    {
        return this->state.Size();
    }
}

#endif /* LIBS_STATE_SYNCHRONIZED_PERSISTENT_STATE_HPP */
