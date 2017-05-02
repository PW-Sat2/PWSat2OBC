#ifndef BASE_ITELEMETRY_CONTAINER_HPP
#define BASE_ITELEMETRY_CONTAINER_HPP

#pragma once

#include <type_traits>
#include "fwd.hpp"

template <typename Type> struct ITelemetryContainer
{
    typedef void (Type::*ReadType)(Reader& reader);
    typedef void (Type::*WriteType)(Writer& reader) const;
    typedef bool (Type::*StateDiscriminator)(const Type& arg) const;

    static_assert(std::is_convertible<decltype(&Type::Read), ReadType>::value,
        "Telemetry type should be able to read its contents from Buffer reader.");

    static_assert(std::is_convertible<decltype(&Type::Write), WriteType>::value,
        "Telemetry type should be able to write its contents to Buffer writer.");

    static_assert(std::is_convertible<decltype(&Type::IsDifferent), StateDiscriminator>::value,
        "Telemetry type should be able to differentiate major/minor state changes.");

    static_assert(Type::Size() > 0, "Telemetry type should report non zero size.");

    static_assert(Type::Id != 0, "Telemetry type should have non zero identifier.");

    virtual const Type& Get() const = 0;

    virtual void Set(const Type& value) = 0;

    virtual void SetVolatile(const Type& value) = 0;
};

#endif
