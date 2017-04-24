#ifndef LIBS_BASE_INCLUDE_TRAITS_HPP_
#define LIBS_BASE_INCLUDE_TRAITS_HPP_

#pragma once

#include <cstdint>
#include <type_traits>
/**
 * @addtogroup utilities
 * @{
 */

/**
 * @brief Type trait for converting bool to integer at compile time.
 */
template <bool> struct AsInt final
{
    /**
     * @brief Assigned value.
     */
    static constexpr std::uint32_t value = 0;
};

/**
 * @brief Type trait for converting bool to integer at compile time.
 *
 * Specialization for true value.
 */
template <> struct AsInt<true> final
{
    /**
     * @brief Assigned value.
     */
    static constexpr std::uint32_t value = 1;
};

/**
 * @brief Type trait for checking whether the passed parameter pack contains at least one argument.
 * @tparam T Input parameter pack.
 */
template <typename... T> struct HasMore final : std::integral_constant<bool, (sizeof...(T) > 0)>
{
};

/**
 * @brief Type trait for counting how many types out of passed parameter pack satisfy passed predicate.
 * @tparam Pred Predicate that should be applied to all types from the pack.
 * @tparam T Parameter pack to inspect.
 */
template <template <typename> class Pred, typename... T> struct CountHelper;

/**
 * @brief Type trait for counting how many types out of passed parameter pack satisfy passed predicate.
 * @tparam Pred Predicate that should be applied to all types from the pack.
 * @tparam Head First type to which the Pred should be applied.
 * @tparam Tail ramaning part of parameter pack.
 */
template <template <typename> class Pred, typename Head, typename... Tail> struct CountHelper<Pred, Head, Tail...> final
{
    /**
     * @brief Current calculation result.
     */
    static constexpr std::uint32_t value = AsInt<Pred<Head>::value>::value + CountHelper<Pred, Tail...>::value;
};

/**
 * @brief Specialization of CountHelper type for empty parameter packs.
 *
 * In other words processing terminator.
 */
template <template <typename> class Pred> struct CountHelper<Pred> final
{
    /**
    * @brief Assigned value.
     */
    static constexpr std::uint32_t value = 0;
};

/**
 * @brief Type trait that applies specific predicate to type T if the condition is met.
 * @tparam T Type of action that should be applied to specified arguments.
 *
 * The applied action should define static method that has signature compatible to this one:
 * @code{.cpp}
 * template <typename... Args> static void Apply(Args&&...)
 * @endcode
 */
template <typename T, bool condition> struct ApplyIf final
{
    /**
     * @brief No operation replacement for applied operation.
     */
    template <typename... Args> static void Apply(Args&&...)
    {
    }
};

template <typename T> struct ApplyIf<T, true> final : T
{
};

/**
 * @brief Returns integer type that is wider than given type
 */
template <typename T> struct Wider;

/**
 * @brief Specialization for uint8_t
 */
template <> struct Wider<std::uint8_t>
{
    /** @brief Returns uint16_t */
    using type = std::uint16_t;
};

/**
 * @brief Specialization for uint16_t
 */
template <> struct Wider<std::uint16_t>
{
    /** @brief Returns uint32_t */
    using type = std::uint32_t;
};

/**
 * @brief Specialization for uint32_t
 */
template <> struct Wider<std::uint32_t>
{
    /** @brief Returns uint64_t */
    using type = std::uint64_t;
};

/**
 * @brief Compile time traits for values
 */
template <typename ValueType> struct ValuesTrait
{
    /**
     * @brief Checks if values is in list
     * @return true if value is in list
     */
    template <ValueType Value, ValueType Head, ValueType... Rest> static constexpr bool IsInList()
    {
        return Value == Head || IsInList<Value, Rest...>();
    }

    /**
     * @brief Checks if values is in list (stop condition)
     * @return Always false
     */
    template <ValueType Value> static constexpr bool IsInList()
    {
        return false;
    }
};

/** @} */
#endif /* LIBS_BASE_INCLUDE_TRAITS_HPP_ */
