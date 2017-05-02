#ifndef BASE_ITELEMETRY_CONTAINER_HPP
#define BASE_ITELEMETRY_CONTAINER_HPP

#pragma once

#include <type_traits>
#include "fwd.hpp"

/**
 * @brief Interface of telemetry container for single telemetry element.
 * @ingroup telemetry
 *
 * This interface should be used to query/update telemetry elements out of global telemetry container.
 * The purpose of this interface is to isolate the telemetry container from the telemetry
 * providers, which are components that are responsible for providing the specific telemetry elements.
 */
template <typename Type> struct ITelemetryContainer
{
    /**
     * @brief Type of pointer to method that is responsible for reading part of the persistent state from
     * its serialized form.
     * @param[in] reader Buffer reader for serialized persistent state.
     */
    typedef void (Type::*ReadType)(Reader& reader);

    /**
     * @brief Type of pointer to method that is responsible for writing part of the persistent state to
     * its serialized form.
     * @param[in] writer Buffer writer for serialized persistent state.
     */
    typedef void (Type::*WriteType)(Writer& reader) const;

    /**
     * @brief Type of pointer to method that is responsible for determination whether two objects of the
     * Type type are considered to be significantly different, meaning the values that they contain
     * are different enough to be considered above usual fluctuations.
     * @param[in] arg Reference to object to compare to.
     * @return True when both objects are not considered almost the same, false otherwise.
     */
    typedef bool (Type::*StateDiscriminator)(const Type& arg) const;

    static_assert(std::is_convertible<decltype(&Type::Read), ReadType>::value,
        "Telemetry type should be able to read its contents from Buffer reader.");

    static_assert(std::is_convertible<decltype(&Type::Write), WriteType>::value,
        "Telemetry type should be able to write its contents to Buffer writer.");

    static_assert(std::is_convertible<decltype(&Type::IsDifferent), StateDiscriminator>::value,
        "Telemetry type should be able to differentiate major/minor state changes.");

    static_assert(Type::Size() > 0, "Telemetry type should report non zero size.");

    static_assert(Type::Id != 0, "Telemetry type should have non zero identifier.");

    /**
     * @brief This procedure returns reference to current telemetry element object.
     * @return Reference to current telemetry element object.
     */
    virtual const Type& Get() const = 0;

    /**
     * @brief This procedure updates single telemetry element object.
     * @param[in] value New telemetry element value.
     *
     * This procedure uses Arg::IsDifferent method to determine whether the new value is different enough
     * to be considered major state change and needs to be saved.
     */
    virtual void Set(const Type& value) = 0;

    /**
     * @brief This procedure updates single telemetry element object.
     * @param[in] value New telemetry element value.
     * This procedure bypasses state change tracking logic and modifies the value regardless
     * of the scale of the change. However as a result changes made by this procedure as considered
     * to be non significant and will never be included in the list of types eligible for saving.
     */
    virtual void SetVolatile(const Type& value) = 0;
};

#endif
