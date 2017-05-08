#ifndef LIBS_STATE_PERSISTENT_STATE_HPP
#define LIBS_STATE_PERSISTENT_STATE_HPP

#pragma once

#include <cstdint>
#include <tuple>
#include <type_traits>
#include "antenna/AntennaConfiguration.hpp"

/**
 * @defgroup persistent_state_details Persistent State Helpers
 * @ingroup persistent_state_container
 *
 * @brief This module contains helper types for persistent state container.
 */

namespace state
{
    namespace details
    {
        /**
         * @brief Declaration of helper type for verification that all Persistent state parts meet its requirements.
         * @ingroup persistent_state_details
         */
        template <typename... Objects> struct CheckObject;

        /**
         * @brief Helper type for verification that all Persistent state parts meet its requirements.
         * @ingroup persistent_state_details
         */
        template <typename Object, typename... Objects> struct CheckObject<Object, Objects...>
        {
            /**
             * @brief Type of pointer to method that is responsible for reading part of the persistent state from
             * its serialized form.
             * @param[in] reader Buffer reader for serialized persistent state.
             */
            typedef void (Object::*ReadType)(Reader& reader);

            /**
             * @brief Type of pointer to method that is responsible for writing part of the persistent state to
             * its serialized form.
             * @param[in] writer Buffer writer for serialized persistent state.
             */
            typedef void (Object::*WriteType)(Writer& reader) const;
            static_assert(std::is_convertible<decltype(&Object::Read), ReadType>::value,
                "Persistent state part should be able to read its contents from Buffer reader.");

            static_assert(std::is_convertible<decltype(&Object::Write), WriteType>::value,
                "Persistent state part should be able to write its contents to Buffer writer.");

            static_assert(Object::Size() > 0, "Persistent state part should report non zero size.");

            /**
             * @brief Verification helper
             */
            static constexpr bool Value = CheckObject<Objects...>::Value;
        };

        /**
         * @brief Persistent state part verification terminator.
         * @ingroup persistent_state_details
         */
        template <> struct CheckObject<>
        {
            /**
             * @brief Verification helper
             */
            static constexpr bool Value = true;
        };
    }

    /**
     * @ingroup persistent_state_container
     * @brief Type that combines together persistent parts of the satellite state.
     * @tparam Parts List of types that are considered important and supposed to be persisted between
     * satellite restarts. There should not be any type duplicates on this list.
     * Every type that is supposed to be part of persistent state should implement interface that is compatible to:
     * @code{.cpp}
     * T();
     * // @brief This function is responsible for reading complete state from the passed buffer reader object.
     * // @param[in] reader buffer reader that should be used to read the serialized state.
     * void Read(Reader& reader);
     *
     * // @brief The Write function is responsible for writing current object state to the passed writer object.
     * // @param[in] writer Buffer writer object that should be used to write the serialized state.
     * void Write(Writer& writer) const;
     *
     * // @brief This procedure is responsible to return size of this object in its serialized form.
     * //
     * // If the size is by definition variable, then this function should return highest possible size
     * // that is possible to be generated.
     * static constexpr std::uint32_t Size();
     * @endcode
     *
     * @tparam StatePolicy Type that provides state tracking capabilities. This type can be used to turn on or off
     * verification whether the Persistent State has been modifed since it has been last read/written.
     * This type should provide interface that is compatible with:
     * @code{.cpp}
     *
     * // @brief Default construction.
     * //
     * // Persistent state policies should be default constructible.
     * T();
     *
     * // @brief This function is used to notify state policy that there has been modification of at least
     * // one persistent state part.
     * void NotifyModified();
     *
     * // @brief This function is used to notify state policy that all changes to the persistent state
     * // have been saved and from now the persistent state should be considered unchanged.
     * void NotifySaved()
     *
     * // @brief This function is used by the persistent state to query the policy whether there have been
     * // any state changes since last state save.
     * // @return True when there has been at least one state modification, false otherwise.
     * bool IsModified() const
     * @endcode
     */
    template <typename StatePolicy, typename... Parts> class PersistentState
    {
      public:
        static_assert(std::is_member_function_pointer<decltype(&StatePolicy::NotifyModified)>::value,
            "StatePolicy should have void NotifyModified() method.");

        static_assert(std::is_member_function_pointer<decltype(&StatePolicy::NotifySaved)>::value,
            "StatePolicy should have void NotifySaved() method.");

        static_assert(std::is_member_function_pointer<decltype(&StatePolicy::IsModified)>::value,
            "StatePolicy should have bool IsModified() method.");

        static_assert(::state::details::CheckObject<Parts...>::Value, "Persistent state part verification failed.");

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
        template <typename... Objects> struct Calculate;

        template <typename Object, typename... Objects> struct Calculate<Object, Objects...>
        {
            static constexpr std::uint32_t Size = Object::Size() + Calculate<Objects...>::Size;
        };

        template <typename Object> struct Calculate<Object>
        {
            static constexpr std::uint32_t Size = Object::Size();
        };

        template <typename Object, typename... Objects>
        static void Write(Writer& reader, //
            const Object& object,         //
            const Objects&... objects     //
            );

        template <typename Object> static void Write(Writer& reader, const Object& object);

        template <typename Object, typename... Objects>
        static void Read(Reader& reader, //
            Object& object,              //
            Objects&... objects          //
            );

        template <typename Object> static void Read(Reader& reader, Object& object);

        std::tuple<Parts...> parts;
        mutable StatePolicy statePolicy;
    };

    template <typename StatePolicy, typename... Parts>
    template <typename Object>
    const Object& PersistentState<StatePolicy, Parts...>::Get() const
    {
        return std::get<Object>(this->parts);
    }

    template <typename StatePolicy, typename... Parts>
    template <typename Object>
    void PersistentState<StatePolicy, Parts...>::Set(const Object& object)
    {
        std::get<Object>(this->parts) = std::move(object);
        statePolicy.NotifyModified();
    }

    template <typename StatePolicy, typename... Parts> void PersistentState<StatePolicy, Parts...>::Read(Reader& reader)
    {
        Read(reader, std::get<Parts>(parts)...);
    }

    template <typename StatePolicy, typename... Parts> void PersistentState<StatePolicy, Parts...>::Write(Writer& writer) const
    {
        Write(writer, std::get<Parts>(parts)...);
    }

    template <typename StatePolicy, typename... Parts>
    template <typename Object, typename... Objects>
    inline void PersistentState<StatePolicy, Parts...>::Write(Writer& writer, //
        const Object& object,                                                 //
        const Objects&... objects                                             //
        )
    {
        object.Write(writer);
        Write(writer, objects...);
    }

    template <typename StatePolicy, typename... Parts>
    template <typename Object>
    inline void PersistentState<StatePolicy, Parts...>::Write(Writer& writer, const Object& object)
    {
        object.Write(writer);
    }

    template <typename StatePolicy, typename... Parts> void PersistentState<StatePolicy, Parts...>::Capture(Writer& writer) const
    {
        Write(writer);
        statePolicy.NotifySaved();
    }

    template <typename StatePolicy, typename... Parts>
    template <typename Object, typename... Objects>
    inline void PersistentState<StatePolicy, Parts...>::Read(Reader& reader, //
        Object& object,                                                      //
        Objects&... objects                                                  //
        )
    {
        object.Read(reader);
        Read(reader, objects...);
    }

    template <typename StatePolicy, typename... Parts>
    template <typename Object>
    inline void PersistentState<StatePolicy, Parts...>::Read(Reader& reader, Object& object)
    {
        object.Read(reader);
    }

    template <typename StatePolicy, typename... Parts> bool PersistentState<StatePolicy, Parts...>::IsModified() const
    {
        return statePolicy.IsModified();
    }

    template <typename StatePolicy, typename... Parts> inline constexpr std::uint32_t PersistentState<StatePolicy, Parts...>::Size()
    {
        return Calculate<Parts...>::Size;
    }
}

#endif
