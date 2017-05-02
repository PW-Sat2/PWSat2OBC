#ifndef LIBS_STATE_TELEMETRY_HPP
#define LIBS_STATE_TELEMETRY_HPP

#pragma once

#include <tuple>
#include "base/ITelemetryContainer.hpp"
#include "base/writer.h"
#include "fwd.hpp"
#include "traits.hpp"

namespace state
{
    namespace details
    {
        template <typename Owner, typename Type> struct TelemetryContainer : public ITelemetryContainer<Type>
        {
            virtual Owner& GetOwner() = 0;

            virtual const Owner& GetOwner() const = 0;

            virtual const Type& Get() const final override;

            virtual void Set(const Type& value) final override;

            virtual void SetVolatile(const Type& value) final override;
        };

        template <typename Owner, typename Type> const Type& TelemetryContainer<Owner, Type>::Get() const
        {
            return GetOwner().template Get<Type>();
        }

        template <typename Owner, typename Type> void TelemetryContainer<Owner, Type>::Set(const Type& value)
        {
            GetOwner().Set(value);
        }

        template <typename Owner, typename Type> void TelemetryContainer<Owner, Type>::SetVolatile(const Type& value)
        {
            GetOwner().SetVolatile(value);
        }

        template <typename... Args> struct UniqueIdVerifier;

        template <typename Base, typename... Args> struct UniqueIdVerifier<Base, Args...>
        {
            static constexpr bool IsUnique = !IsValueInList<int>::IsInList<Base::Id, Args::Id...>() && UniqueIdVerifier<Args...>::IsUnique;
        };

        template <typename Base> struct UniqueIdVerifier<Base>
        {
            static constexpr bool IsUnique = true;
        };

        template <typename... Args> struct TotalSize;

        template <typename Base, typename... Args> struct TotalSize<Base, Args...>
        {
            static constexpr int Value = Base::Size() + TotalSize<Args...>::Value;
        };

        template <typename Base> struct TotalSize<Base>
        {
            static constexpr int Value = Base::Size();
        };
    }

    /**
     * @brief Class that is responsible for keeping together complete satellite telemetry information.
     * @ingroup StateDef
     */
    template <typename... Type> class Telemetry final : public details::TelemetryContainer<Telemetry<Type...>, Type>...
    {
      public:
        static_assert(AreTypesUnique<Type...>::value, "Telemetry types should be unique");

        static_assert(details::UniqueIdVerifier<Type...>::IsUnique, "Telemetry type identifiers should be unique");

        static constexpr int TypeCount = sizeof...(Type);

        static constexpr int PayloadSize = details::TotalSize<Type...>::Value;

        static constexpr int TotalSerializedSize = PayloadSize + 2 * TypeCount * sizeof(std::uint8_t);

        virtual Telemetry<Type...>& GetOwner() final override;

        virtual const Telemetry<Type...>& GetOwner() const final override;

        template <typename Arg> void Set(const Arg& arg);

        template <typename Arg> void SetVolatile(const Arg& arg);

        template <typename Arg> const Arg& Get() const;

        bool IsModified() const;

        void WriteModified(Writer& writer) const;

        void CommitCapture();

      private:
        template <typename Arg> bool IsDifferent(const Arg& arg) const;

        template <typename Arg> using ElementContainer = std::pair<Arg, bool>;

        typedef std::tuple<ElementContainer<Type>...> Container;

        template <int Tag, typename T, typename... Args> bool IsModifiedInternal() const;

        template <int Tag> bool IsModifiedInternal() const;

        template <int Tag, typename T, typename... Args> void WriteModifiedInternal(Writer& writer) const;

        template <int Tag> void WriteModifiedInternal(Writer& writer) const;

        template <int Tag, typename T, typename... Args> void CommitCaptureInternal();

        template <int Tag> void CommitCaptureInternal();

        Container storage;
    };

    template <typename... Type> constexpr int Telemetry<Type...>::TypeCount;
    template <typename... Type> constexpr int Telemetry<Type...>::PayloadSize;
    template <typename... Type> constexpr int Telemetry<Type...>::TotalSerializedSize;

    template <typename... Type> Telemetry<Type...>& Telemetry<Type...>::GetOwner()
    {
        return *this;
    }

    template <typename... Type> const Telemetry<Type...>& Telemetry<Type...>::GetOwner() const
    {
        return *this;
    }

    template <typename... Type> template <typename Arg> void Telemetry<Type...>::Set(const Arg& arg)
    {
        if (IsDifferent(arg))
        {
            auto& entry = std::get<ElementContainer<Arg>>(this->storage);
            entry.first = arg;
            entry.second = true;
        }
    }

    template <typename... Type> template <typename Arg> void Telemetry<Type...>::SetVolatile(const Arg& arg)
    {
        if (IsDifferent(arg))
        {
            std::get<ElementContainer<Arg>>(this->storage).first = arg;
        }
    }

    template <typename... Type> template <typename Arg> const Arg& Telemetry<Type...>::Get() const
    {
        return std::get<ElementContainer<Arg>>(this->storage).first;
    }

    template <typename... Type> template <typename Arg> bool Telemetry<Type...>::IsDifferent(const Arg& arg) const
    {
        return std::get<ElementContainer<Arg>>(this->storage).first.IsDifferent(arg);
    }

    template <typename... Type> inline bool Telemetry<Type...>::IsModified() const
    {
        return IsModifiedInternal<0, Type...>();
    }

    template <typename... Type> template <int Tag, typename T, typename... Args> inline bool Telemetry<Type...>::IsModifiedInternal() const
    {
        return std::get<ElementContainer<T>>(this->storage).second || IsModifiedInternal<0, Args...>();
    }

    template <typename... Type> template <int Tag> inline bool Telemetry<Type...>::IsModifiedInternal() const
    {
        return false;
    }

    template <typename... Type> inline void Telemetry<Type...>::WriteModified(Writer& writer) const
    {
        return WriteModifiedInternal<0, Type...>(writer);
    }

    template <typename... Type> inline void Telemetry<Type...>::CommitCapture()
    {
        CommitCaptureInternal<0, Type...>();
    }

    template <typename... Type>
    template <int Tag, typename T, typename... Args>
    inline void Telemetry<Type...>::WriteModifiedInternal(Writer& writer) const
    {
        const auto& entry = std::get<ElementContainer<T>>(this->storage);
        if (entry.second)
        {
            writer.WriteByte(T::Id);
            entry.first.Write(writer);
            writer.WriteByte(T::Id);
        }

        WriteModifiedInternal<0, Args...>(writer);
    }

    template <typename... Type> template <int Tag> inline void Telemetry<Type...>::WriteModifiedInternal(Writer& /*writer*/) const
    {
    }

    template <typename... Type> template <int Tag, typename T, typename... Args> inline void Telemetry<Type...>::CommitCaptureInternal()
    {
        std::get<ElementContainer<T>>(this->storage).second = false;
        CommitCaptureInternal<0, Args...>();
    }

    template <typename... Type> template <int Tag> inline void Telemetry<Type...>::CommitCaptureInternal()
    {
    }
}

#endif
