#ifndef SRC_BASE_UTILS_H
#define SRC_BASE_UTILS_H

#pragma once

#include <stdbool.h>
#include <chrono>
#include <cstdint>
#include <type_traits>
#include <utility>

/**
 * @brief Converts bool value to 1 or 0
 * @param[in] value Value to convert
 * @return 1 for true, 0 for false
 */
static constexpr inline int ToInt(bool value)
{
    return value ? 1 : 0;
}

/**
 * @brief Just marker indicating that value is in bytes
 * @param[in] value Value
 * @return The same value
 */
constexpr std::size_t operator"" _Bytes(unsigned long long int value)
{
    return value;
}

/**
 * @brief Converts value in kilobytes to bytes
 * @param[in] value Values in kilobytes
 * @return Value in bytes
 */
constexpr std::size_t operator"" _KB(unsigned long long int value)
{
    return value * 1024_Bytes;
}

/**
 * @brief Converts value in megabytes to bytes
 * @param[in] value Values in megabytes
 * @return Value in bytes
 */
constexpr std::size_t operator"" _MB(unsigned long long int value)
{
    return value * 1024_KB;
}

/**
 * @brief Just marker indicating that value is in Hz
 * @param[in] value Value in Hz
 * @return the same value
 */
constexpr std::uint32_t operator"" _Hz(unsigned long long int value)
{
    return value;
}

/**
 * @brief Converts KHz to Hz
 * @param[in] value Value in KHz
 * @return Value in Hz
 */
constexpr std::uint32_t operator"" _KHz(unsigned long long int value)
{
    return value * 1000_Hz;
}

/**
 * @brief Converts MHz to Hz
 * @param[in] value Value in MHz
 * @return Value in Hz
 */
constexpr std::uint32_t operator"" _MHz(unsigned long long int value)
{
    return value * 1000_KHz;
}

/**
 * @brief Inheriting from this class, will make derived class unconstructable
 */
struct PureStatic
{
    PureStatic() = delete;
};

/**
 * @brief Class that can logically contain either a value of type T or no value.
 */
template <class T> class Option
{
  public:
    /**
     * @brief Default ctor
     */
    Option();

    /**
     * @brief Factory method that constructs empty Option instance.
     * @return Empty Option instance.
     */
    static Option<T> None()
    {
        return Option<T>(false, T());
    }

    /**
     * @brief Factory method that constucts Option instance and its value in place
     * @param args Arguments passed to constructor of T
     * @return Option instance
     */
    template <typename... Args> static Option<T> Some(Args&&... args)
    {
        return Option<T>(true, std::forward<Args>(args)...);
    }

    /**
     * @brief Factory method that constructs Option instance that holds given value.
     * @param[in] value Value to hold in Option instance.
     * @return Option instance that holds a value.
     */
    static Option<T> Some(T& value)
    {
        return Option<T>(true, value);
    }

    /**
      * @brief A flag indicating if this Option instance holds a value.
      */
    bool HasValue;

    /**
      * @brief Value held by Option instance if HasValue is true.
      */
    T Value;

  private:
    Option(bool hasValue, T&& value) : HasValue(hasValue), Value(std::move(value))
    {
    }

    Option(bool hasValue, T& value) : HasValue(hasValue), Value(value)
    {
    }
};

template <class T> inline Option<T>::Option() : HasValue(false), Value(T())
{
}

/**
 * @brief Factory method that constructs empty Option instance.
 * @return Empty Option instance.
 */
template <typename T> static inline Option<T> None()
{
    return Option<T>::None();
}

/**
 * @brief Factory method that constructs Option instance that holds given value.
 * @param[in] value Value to hold in Option instance.
 * @return Option instance that holds a value.
 */
template <typename T> static inline Option<std::remove_reference_t<T>> Some(T&& value)
{
    using U = std::remove_reference_t<T>;

    return Option<U>::Some(std::forward<U>(value));
}

/**
 * @brief Equality operator for @ref Option<T>
 * @param lhs Left side value
 * @param rhs Right side value (raw, not option)
 * @retval false lhs is None
 * @retval false lhs is Some and rhs is not equal to holded value
 * @retval true lhs is Some and rhs is equal to holded value
 */
template <typename T> static inline bool operator==(const Option<T>& lhs, const T& rhs)
{
    if (!lhs.HasValue)
    {
        return false;
    }

    return lhs.Value == rhs;
}

/**
 * @brief Equality operator for @ref Option<T>
 * @param lhs Right side value (raw, not option)
 * @param rhs Left side value
 * @retval false rhs is None
 * @retval false rhs is Some and lhs is not equal to holded value
 * @retval true rhs is Some and lhs is equal to holded value
 */
template <typename T> bool operator==(const T& lhs, const Option<T>& rhs)
{
    return rhs == lhs;
}

/**
 * @brief Inequality operator for @ref Option<T>
 * @param lhs Left side value
 * @param rhs Right side value (raw, not option)
 * @retval true lhs is None
 * @retval true lhs is Some and rhs is not equal to holded value
 * @retval false lhs is Some and rhs is equal to holded value
 */
template <typename T> inline bool operator!=(const Option<T>& lhs, const T& rhs)
{
    return !(lhs == rhs);
}

/**
 * @brief Equality operator for @ref Option<T>
 * @brief Inequality operator for @ref Option<T>
 * @param lhs Left side value
 * @param rhs Right side value (option)
 * @retval true when both are None
 * @retval false when one is None and other is Some
 * @retval false when both are Some and underlying values are not equal
 * @retval true when both are Some and underlying values are equal
 */
template <typename T> bool operator==(const Option<T>& lhs, const Option<T>& rhs)
{
    if (lhs.HasValue && rhs.HasValue)
    {
        return lhs.Value == rhs.Value;
    }

    return lhs.HasValue == rhs.HasValue;
}

/**
 * @brief Equality operator for @ref Option<T>
 * @param lhs Left side value
 * @param rhs Right side value (option)
 * @retval true when both are None
 * @retval false when one is None and other is Some
 * @retval false when both are Some and underlying values are not equal
 * @retval true when both are Some and underlying values are equal
 */
template <typename T> inline bool operator!=(const Option<T>& lhs, const Option<T>& rhs)
{
    return !(lhs == rhs);
}

/**
 * @brief Private-inherit this class to prevent copy-operations
 */
struct NotCopyable
{
    NotCopyable() = default;
    NotCopyable(const NotCopyable& arg) = delete;
    NotCopyable& operator=(const NotCopyable& arg) = delete;
};

/**
 * @brief Private-inherit this class to prevent move-operations
 */
struct NotMoveable
{
    NotMoveable() = default;
    NotMoveable(NotMoveable&& arg) = delete;
    NotMoveable& operator=(NotMoveable&& arg) = delete;
};

constexpr auto MaxValueOnBits(std::uint8_t bitsCount)
{
    return (1 << bitsCount) - 1;
}

/**
 * @brief Interface for callback objects that will receive ticks.
 */
struct TimeAction
{
    /**
     * @brief Method that will be called by BURTC.
     * @param[in] interval Interval that passed since last tick
     */
    void virtual Invoke(std::chrono::milliseconds interval) = 0;
};

/**
 * @brief Holds action (lambda) that will be invoked on destruction unless explictly skipped
 */
template <typename Action> class OnLeaveAction final : private NotCopyable
{
  public:
    /**
     * @brief Ctor
     * @param action Action to invoke on destruction
     */
    OnLeaveAction(Action action);

    /**
     * @brief Move ctor
     * @param other Other
     */
    OnLeaveAction(OnLeaveAction<Action>&& other) noexcept;

    /**
     * @brief Destructor
     */
    ~OnLeaveAction();

    /**
     * @brief Skips action invocation
     */
    void Skip();

  private:
    /** @brief Action to invoke */
    Action _action;
    /** @brief Flag indicating if action should be skipped */
    bool _skip;
};

template <typename Action> OnLeaveAction<Action> OnLeave(Action action)
{
    return {action};
}

template <typename Action> OnLeaveAction<Action>::OnLeaveAction(Action action) : _action(action), _skip(false)
{
}

template <typename Action>
OnLeaveAction<Action>::OnLeaveAction(OnLeaveAction<Action>&& other) noexcept : _action(std::move(other._action)), _skip(other._skip)
{
    other._skip = true;
}

template <typename Action> OnLeaveAction<Action>::~OnLeaveAction()
{
    if (!this->_skip)
    {
        this->_action();
    }
}

template <typename Action> void OnLeaveAction<Action>::Skip()
{
    this->_skip = true;
}

/**
 * @brief Type wrapping integer of non-standard size
 * @tparam Integer type that will be used to hold smaller value
 * @tparam BitsCount Number of bits used by number
 */
template <typename Underlying, std::uint8_t BitsCount> class BitValue
{
  public:
    /** @brief Mask used to trim value */
    static constexpr Underlying Mask = ~(~0ull << BitsCount);

    /** @brief Default ctor */
    constexpr BitValue();
    /**
     * @brief Initializes with given value
     * @param v Initial value
     */
    constexpr BitValue(Underlying v);

    /**
     * @brief Extracts held value
     */
    inline operator Underlying() const;

    /**
     * @brief Extracts held value
     * @return Held value
     */
    inline Underlying Value() const;

    /**
     * @brief Default assignment operator
     * @param other Other BitValue
     * @return Reference to this BitValue
     */
    constexpr BitValue<Underlying, BitsCount>& operator=(const BitValue<Underlying, BitsCount>& other) = default;

  private:
    /** @brief Held value */
    Underlying _value;
};

template <typename Underlying, std::uint8_t BitsCount> BitValue<Underlying, BitsCount>::operator Underlying() const
{
    return this->_value;
}

template <typename Underlying, std::uint8_t BitsCount> constexpr BitValue<Underlying, BitsCount>::BitValue(Underlying v) : _value(v & Mask)
{
}

template <typename Underlying, std::uint8_t BitsCount> constexpr BitValue<Underlying, BitsCount>::BitValue() : _value(0)
{
}

template <typename Underlying, std::uint8_t BitsCount> inline Underlying BitValue<Underlying, BitsCount>::Value() const
{
    return this->_value;
}

template <typename Underlying, std::uint8_t BitsCount>
inline bool operator<(BitValue<Underlying, BitsCount>& lhs, BitValue<Underlying, BitsCount>& rhs)
{
    return lhs.Value() < rhs.Value();
}

template <typename Underlying, std::uint8_t BitsCount>
inline bool operator>(BitValue<Underlying, BitsCount>& lhs, BitValue<Underlying, BitsCount>& rhs)
{
    return rhs < lhs;
}

template <typename Underlying, std::uint8_t BitsCount>
inline bool operator<=(BitValue<Underlying, BitsCount>& lhs, BitValue<Underlying, BitsCount>& rhs)
{
    return !(lhs > rhs);
}

template <typename Underlying, std::uint8_t BitsCount>
inline bool operator>=(BitValue<Underlying, BitsCount>& lhs, BitValue<Underlying, BitsCount>& rhs)
{
    return !(lhs < rhs);
}

template <typename Underlying, std::uint8_t BitsCount>
inline bool operator==(BitValue<Underlying, BitsCount>& lhs, BitValue<Underlying, BitsCount>& rhs)
{
    return lhs.Value() == rhs.Value();
}

template <typename Underlying, std::uint8_t BitsCount>
inline bool operator!=(BitValue<Underlying, BitsCount>& lhs, BitValue<Underlying, BitsCount>& rhs)
{
    return !(lhs == rhs);
}

/** @brief 12-bit unsigned int */
using uint12_t = BitValue<std::uint16_t, 12>;
/** @brief 10-bit unsigned int */
using uint10_t = BitValue<std::uint16_t, 10>;

#endif
