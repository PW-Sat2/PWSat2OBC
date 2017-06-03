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
 * @brief Checks if type is in list
 * @tparam T Type to check
 * @tparam List Template pack for checking
 */
template <typename T, typename... List> class IsInList
{
  private:
    /**
     * @brief Check logic - single step
     * @return Value indicating whether type is in list
     */
    template <std::uint8_t Tag, typename Head, typename... Rest> static constexpr bool Check()
    {
        if (std::is_same<T, Head>::value)
        {
            return true;
        }

        return Check<0, Rest...>();
    }

    /**
     * @brief Check logic - stop condition
     * @return Always false
     */
    template <std::uint8_t Tag> static constexpr bool Check()
    {
        return false;
    }

  public:
    /** @brief Value indicating whether type is in list */
    static constexpr bool value = Check<0, List...>();
};

/**
 * @brief Checks if types in template pack are unique
 * @tparam Ts Template pack to check
 */
template <typename... Ts> class AreTypesUnique
{
  private:
    /**
     * @brief Check logic - single step
     * @return Value indicating whether types in template pack are unique
     */
    template <std::uint8_t Tag, typename Head, typename... Rest> static constexpr bool IsUnique()
    {
        if (IsInList<Head, Rest...>::value)
        {
            return false;
        }

        return IsUnique<0, Rest...>();
    }

    /**
     * @brief Check logic - stop condition
     * @return Always true
     */
    template <std::uint8_t Tag> static constexpr bool IsUnique()
    {
        return true;
    }

  public:
    /** @brief Value indicating whether types in template pack are unique */
    static constexpr bool value = IsUnique<0, Ts...>();
};

/**
 * @brief Checks if value is in list
 */
template <typename ValueType> struct IsValueInList
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

/**
 * @brief Trait that can be used to determine whether passed type contains Write method
 * that accepts requested parameters.
 * @tparam Type Queried type
 * @tparam Args Parameter pack that contains types of Write method arguments.
 */
template <typename Type, typename... Args> class HasWrite
{
    template <typename C, typename = decltype(std::declval<C>().Write(std::declval<Args>()...))> static std::true_type test(int);

    template <typename C> static std::false_type test(...);

  public:
    /**
     * @brief Flag indicating whether the Type type contains Write(Args...) method.
     *
     * True on success, false otherwise.
     */
    static constexpr bool Value = decltype(test<Type>(0))::value;
};

/**
 * @brief Trait that can be used to determine whether passed type contains Read method
 * that accepts requested parameters.
 * @tparam Type Queried type
 * @tparam Args Parameter pack that contains types of Read method arguments.
 */
template <typename Type, typename... Args> class HasRead
{
    template <typename C, typename = decltype(std::declval<C>().Read(std::declval<Args>()...))> static std::true_type test(int);

    template <typename C> static std::false_type test(...);

  public:
    /**
     * @brief Flag indicating whether the Type type contains Read(Args...) method.
     *
     * True on success, false otherwise.
     */
    static constexpr bool Value = decltype(test<Type>(0))::value;
};

/**
 * @brief Traits that can be used to determine whether passed type contains Initialize method
 * that accepts requested parameters.
 * @tparam Type Queried type
 * @tparam Args Parameter pack that contains types of Read method arguments.
 */
template <typename Type, typename... Args> class HasInitialize
{
    template <typename C, typename = decltype(std::declval<C>().Initialize(std::declval<Args>()...))> static std::true_type test(int);

    template <typename C> static std::false_type test(...);

  public:
    /**
     * @brief Type indicating whether the Type type contains Initialize(Args...) method.
     *
     * std::true_type on success, std::false_type otherwise.
     */
    typedef decltype(test<Type>(0)) ValueType;

    /**
     * @brief Flag indicating whether the Type type contains Initialize(Args...) method.
     *
     * True on success, false otherwise.
     */
    static constexpr bool Value = ValueType::value;
};

/** @} */
#endif /* LIBS_BASE_INCLUDE_TRAITS_HPP_ */
