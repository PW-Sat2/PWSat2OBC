#ifndef LIBS_STATE_TELEMETRY_HPP
#define LIBS_STATE_TELEMETRY_HPP

#pragma once

#include <tuple>
#include "base/ITelemetryContainer.hpp"
#include "fwd.hpp"
#include "traits.hpp"

namespace state
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

    template <typename... Type> class Telemetry final : public TelemetryContainer<Telemetry<Type...>, Type>...
    {
      public:
        static_assert(AreTypesUnique<Type...>::value, "Telemetry types should be unique");

        template <typename... Args> struct UniqueIdVerifier;

        template <typename Base, typename... Args> struct UniqueIdVerifier<Base, Args...>
        {
            static constexpr bool IsUnique = !IsValueInList<int>::IsInList<Base::Id, Args::Id...>() && UniqueIdVerifier<Args...>::IsUnique;
        };

        template <typename Base> struct UniqueIdVerifier<Base>
        {
            static constexpr bool IsUnique = true;
        };

        static_assert(UniqueIdVerifier<Type...>::IsUnique, "Telemetry type identifiers should be unique");

        virtual Telemetry<Type...>& GetOwner() final override;

        virtual const Telemetry<Type...>& GetOwner() const final override;

        template <typename Arg> void Set(const Arg& arg);

        template <typename Arg> void SetVolatile(const Arg& arg);

        template <typename Arg> const Arg& Get() const;

      private:
        template <typename Arg> bool IsDifferent(const Arg& arg) const;

        template <typename Arg> using Container = std::pair<Arg, bool>;

        std::tuple<Container<Type>...> storage;
    };

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
            auto& entry = std::get<Container<Arg>>(storage);
            entry.first = arg;
            entry.second = true;
        }
    }

    template <typename... Type> template <typename Arg> void Telemetry<Type...>::SetVolatile(const Arg& arg)
    {
        if (IsDifferent(arg))
        {
            std::get<Container<Arg>>(storage).first = arg;
        }
    }

    template <typename... Type> template <typename Arg> const Arg& Telemetry<Type...>::Get() const
    {
        return std::get<Container<Arg>>(storage).first;
    }

    template <typename... Type> template <typename Arg> bool Telemetry<Type...>::IsDifferent(const Arg& arg) const
    {
        return std::get<Container<Arg>>(this->storage).first.IsDifferent(arg);
    }
}

#endif
