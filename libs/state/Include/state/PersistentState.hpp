#ifndef LIBS_STATE_PERSISTENT_STATE_HPP
#define LIBS_STATE_PERSISTENT_STATE_HPP

#pragma once

#include <cstdint>
#include <tuple>
#include "antenna/AntennaConfiguration.hpp"

namespace state
{
    template <typename... Parts> class PersistentState
    {
      public:
        template <typename Object> const Object& Get() const;

        template <typename Object> void Set(Object object);

        void Read(Reader& reader);

        void Write(Writer& writer) const;

        void Capture(Writer& writer) const;

        bool IsModified() const;

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
