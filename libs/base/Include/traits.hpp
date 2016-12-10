#ifndef LIBS_BASE_INCLUDE_TRAITS_HPP_
#define LIBS_BASE_INCLUDE_TRAITS_HPP_

#pragma once

#include <cstdint>
#include <type_traits>

template <bool> struct AsInt final
{
    static constexpr std::uint32_t value = 0;
};

template <> struct AsInt<true> final
{
    static constexpr std::uint32_t value = 1;
};

template <typename... T> struct HasMore final : std::integral_constant<bool, (sizeof...(T) > 0)>
{
};

template <template <typename> class Pred, typename... T> struct CountHelper;

template <template <typename> class Pred, typename Head, typename... Tail> struct CountHelper<Pred, Head, Tail...> final
{
    static constexpr std::uint32_t value = AsInt<Pred<Head>::value>::value + CountHelper<Pred, Tail...>::value;
};

template <template <typename> class Pred> struct CountHelper<Pred> final
{
    static constexpr std::uint32_t value = 0;
};

template <typename T, bool condition> struct ApplyIf final
{
    template <typename... Args> static void Apply(Args&&...)
    {
    }
};

template <typename T> struct ApplyIf<T, true> final : T
{
};

#endif /* LIBS_BASE_INCLUDE_TRAITS_HPP_ */
