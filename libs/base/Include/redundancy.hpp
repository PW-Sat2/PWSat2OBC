#ifndef LIBS_BASE_INCLUDE_REDUNDANCY_HPP_
#define LIBS_BASE_INCLUDE_REDUNDANCY_HPP_

#include <gsl/span>
#include "utils.h"

namespace redundancy
{
    /**
     * @defgroup redundancy Redundancy support utilities
     * @{
     */

    /**
     * @brief Votes the actual value based on 3 provided inputs.
     * @param[in] a First input
     * @param[in] b Second input
     * @param[in] c Third input
     * @tparam T Type used for majority vote. Must implement == operation.
     * @return Value based on majority vote, or None if all parameters are different.
     */
    template <typename T> static Option<T> Vote(T a, T b, T c)
    {
        if (a == b || a == c)
            return Some(a);
        if (b == c)
            return Some(b);

        return None<T>();
    }

    /**
     * @brief Performs bitwise majority vote based on three inputs.
     * @param[in] a First input
     * @param[in] b Second input
     * @param[in] c Third input
     * @tparam T Type used for majority vote. Must be integral or enum type
     * @return Value calculated from bitwise majority vote.
     */
    template <typename T> T Correct(T a, T b, T c)
    {
        static_assert(std::is_integral<T>::value || std::is_enum<T>::value, "Correction requires integral or enum type");
        return (a & b) | (b & c) | (a & c);
    }

    template <typename T> T Correct(T a, T b, T c, T d, T e)
    {
        static_assert(std::is_integral<T>::value || std::is_enum<T>::value, "Correction requires integral or enum type");
        return (c & d & e) | (b & d & e) | (b & c & e) | (b & c & d) | (a & d & e) | (a & c & e) | (a & c & d) | (a & b & e) | (a & b & d) |
            (a & b & c);
    }

    /**
     * @brief Performs bitwise majority vote based on three elements in array.
     * @param[in] elements Array with three elements
     * @tparam T Type used for majority vote. Must be integral or enum type
     * @return Value calculated from bitwise majority vote.
     */
    template <typename T> inline T Correct(std::array<T, 3>& elements)
    {
        return Correct(elements[0], elements[1], elements[2]);
    }

    /**
     * @brief Performs bitwise majority votes on entire data buffers.
     * @param[in,out] buffer1 First input
     * @param[in] buffer2 Second input
     * @param[in] buffer3 Third input
     * @tparam T Type used for majority vote. Must be integral or enum type
     * @return True if all buffers are of the same size, False otherwise.
     */
    template <typename T> bool CorrectBuffer(gsl::span<T> buffer1, gsl::span<T> buffer2, gsl::span<T> buffer3)
    {
        static_assert(std::is_integral<T>::value || std::is_enum<T>::value, "Correction requires integral or enum type");
        if (buffer1.length() != buffer2.length() || buffer2.length() != buffer3.length())
            return false;

        if (buffer1.length() % 4 != 0)
        {
            return false;
        }

        auto length = buffer1.length() / sizeof(std::uint32_t);

        auto a = reinterpret_cast<std::uint32_t*>(buffer1.data());
        auto b = reinterpret_cast<const std::uint32_t*>(buffer2.data());
        auto c = reinterpret_cast<const std::uint32_t*>(buffer3.data());

        if (!(IsAligned<4>(a) && IsAligned<4>(b) && IsAligned<4>(c)))
        {
            return false;
        }

        for (decltype(length) i = 0; i < length; ++i)
        {
            a[i] = Correct(a[i], b[i], c[i]);
        }

        return true;
    }

    template <typename T> bool CorrectBuffer(gsl::span<T> result, const std::array<gsl::span<const T>, 5>& buffers)
    {
        static_assert(std::is_integral<T>::value || std::is_enum<T>::value, "Correction requires integral or enum type");
        if (buffers[0].length() != buffers[1].length() || buffers[1].length() != buffers[2].length() ||
            buffers[2].length() != buffers[3].length() || buffers[3].length() != buffers[4].length() ||
            result.length() != buffers[0].length())
            return false;

        if (result.length() % 4 != 0)
        {
            return false;
        }

        auto length = result.length() / sizeof(std::uint32_t);

        auto r = reinterpret_cast<std::uint32_t*>(result.data());

        auto a = reinterpret_cast<const std::uint32_t*>(buffers[0].data());
        auto b = reinterpret_cast<const std::uint32_t*>(buffers[1].data());
        auto c = reinterpret_cast<const std::uint32_t*>(buffers[2].data());
        auto d = reinterpret_cast<const std::uint32_t*>(buffers[3].data());
        auto e = reinterpret_cast<const std::uint32_t*>(buffers[4].data());

        if (!(IsAligned<4>(a) && IsAligned<4>(b) && IsAligned<4>(c) && IsAligned<4>(d) && IsAligned<4>(e)))
        {
            return false;
        }

        for (decltype(length) i = 0; i < length; ++i)
        {
            r[i] = Correct(a[i], b[i], c[i], d[i], e[i]);
        }

        return true;
    }

    /**
     * @brief Performs bitwise majority votes on entire data buffers.
     * @param[out] output Buffer for corrected result
     * @param[in] buffer1 First input
     * @param[in] buffer2 Second input
     * @param[in] buffer3 Third input
     * @tparam T Type used for majority vote. Must be integral or enum type
     * @return True if all buffers are of the same size, False otherwise.
     */
    template <typename T>
    bool CorrectBuffer(gsl::span<std::remove_cv_t<T>> output, gsl::span<T> buffer1, gsl::span<T> buffer2, gsl::span<T> buffer3)
    {
        static_assert(std::is_integral<T>::value || std::is_enum<T>::value, "Correction requires integral or enum type");
        if (output.length() != buffer1.length() || buffer1.length() != buffer2.length() || buffer2.length() != buffer3.length())
            return false;

        if (output.length() % 4 != 0)
        {
            return false;
        }

        auto length = output.length() / sizeof(std::uint32_t);

        auto r = reinterpret_cast<std::uint32_t*>(output.data());

        auto a = reinterpret_cast<const std::uint32_t*>(buffer1.data());
        auto b = reinterpret_cast<const std::uint32_t*>(buffer2.data());
        auto c = reinterpret_cast<const std::uint32_t*>(buffer3.data());

        if (!(IsAligned<4>(a) && IsAligned<4>(b) && IsAligned<4>(c)))
        {
            return false;
        }

        for (decltype(length) i = 0; i < length; ++i)
        {
            r[i] = Correct(a[i], b[i], c[i]);
        }

        return true;
    }

    /** @} */
}

#endif /* LIBS_BASE_INCLUDE_REDUNDANCY_HPP_ */
