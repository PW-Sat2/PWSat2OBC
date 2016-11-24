#ifndef SRC_BASE_UTILS_H
#define SRC_BASE_UTILS_H

#pragma once

#include <stdbool.h>
#include <cstdint>

static inline int ToInt(bool value)
{
    return value ? 1 : 0;
}

constexpr std::size_t operator"" _Bytes(unsigned long long int value)
{
    return value;
}

constexpr std::size_t operator"" _KB(unsigned long long int value)
{
    return value * 1024_Bytes;
}

constexpr std::size_t operator"" _MB(unsigned long long int value)
{
    return value * 1024_KB;
}

/**
 * Inheriting from this class, will make derived class unconstructable
 */
struct PureStatic
{
    PureStatic() = delete;
};

#endif
