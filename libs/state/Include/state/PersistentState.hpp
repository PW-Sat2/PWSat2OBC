#ifndef LIBS_STATE_PERSISTENT_STATE_HPP
#define LIBS_STATE_PERSISTENT_STATE_HPP

#pragma once

#include <cstdint>
#include <tuple>
#include "antenna/AntennaConfiguration.hpp"

namespace state
{
    /**
     * @ingroup StateDef
     * @brief Type that combines together persistent parts of the satellite state.
     * @tparam Parts List of types that are considered important and supposed to be persisted between
     * satellite restarts. There should not be any type duplicates on this list.
     *
     * Every type that is supposed to be part of persistent state should implement interface that is compatible to:
     * @code{.cpp}
     * T();
     * void Read(Reader& reader);
     * void Write(Writer& writer) const;
     * static constexpr std::uint32_t Size();
     * @endcode
     */
    template <typename... Parts> class PersistentState
    {
      public:
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
        template <typename Object> void Set(Object object);

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
        mutable bool modified = false;
    };

    template <typename... Parts> template <typename Object> const Object& PersistentState<Parts...>::Get() const
    {
        return std::get<Object>(this->parts);
    }

    template <typename... Parts> template <typename Object> void PersistentState<Parts...>::Set(Object object)
    {
        std::get<Object>(this->parts) = std::move(object);
        this->modified = true;
    }

    template <typename... Parts> void PersistentState<Parts...>::Read(Reader& reader)
    {
        Read(reader, std::get<Parts>(parts)...);
    }

    template <typename... Parts> void PersistentState<Parts...>::Write(Writer& writer) const
    {
        Write(writer, std::get<Parts>(parts)...);
    }

    template <typename... Parts>
    template <typename Object, typename... Objects>
    inline void PersistentState<Parts...>::Write(Writer& writer, //
        const Object& object,                                    //
        const Objects&... objects                                //
        )
    {
        object.Write(writer);
        Write(writer, objects...);
    }

    template <typename... Parts>
    template <typename Object>
    inline void PersistentState<Parts...>::Write(Writer& writer, const Object& object)
    {
        object.Write(writer);
    }

    template <typename... Parts> void PersistentState<Parts...>::Capture(Writer& writer) const
    {
        Write(writer);
        this->modified = false;
    }

    template <typename... Parts>
    template <typename Object, typename... Objects>
    inline void PersistentState<Parts...>::Read(Reader& reader, //
        Object& object,                                         //
        Objects&... objects                                     //
        )
    {
        object.Read(reader);
        Read(reader, objects...);
    }

    template <typename... Parts> template <typename Object> inline void PersistentState<Parts...>::Read(Reader& reader, Object& object)
    {
        object.Read(reader);
    }

    template <typename... Parts> bool PersistentState<Parts...>::IsModified() const
    {
        return this->modified;
    }

    template <typename... Parts> inline constexpr std::uint32_t PersistentState<Parts...>::Size()
    {
        return Calculate<Parts...>::Size;
    }
}

#endif
