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

        for (auto i = 0; i < buffer1.length(); ++i)
        {
            buffer1[i] = Correct(buffer1[i], buffer2[i], buffer3[i]);
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

        for (auto i = 0; i < buffer1.length(); ++i)
        {
            output[i] = Correct(buffer1[i], buffer2[i], buffer3[i]);
        }

        return true;
    }

    /** @} */
}

#endif /* LIBS_BASE_INCLUDE_REDUNDANCY_HPP_ */
