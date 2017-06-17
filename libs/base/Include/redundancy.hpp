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
    * @brief Performs bitwise majority vote based on five inputs.
    * @param[in] a First input
    * @param[in] b Second input
    * @param[in] c Third input
    * @param[in] d Fourth input
    * @param[in] e Fifth input
    * @tparam T Type used for majority vote. Must be integral or enum type
    * @return Value calculated from bitwise majority vote.
    */
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
     * @return True if all buffers are valid, False otherwise
     * @remark All buffer must have length that is multiply of 4 and be aligned to 4 bytes
     */
    bool CorrectBuffer(gsl::span<std::uint8_t> buffer1, gsl::span<const std::uint8_t> buffer2, gsl::span<const std::uint8_t> buffer3);

    /**
     * @brief Performs bitwise majority voting on entire data buffer
     * @param result Buffer for result
     * @param buffers Array for five spans containing inputs
     * @return True if all buffers are valid, False otherwise
     * @remark All buffer must have length that is multiply of 4 and be aligned to 4 bytes
     */
    bool CorrectBuffer(gsl::span<std::uint8_t> result, const std::array<gsl::span<const std::uint8_t>, 5>& buffers);

    /**
     * @brief Performs bitwise majority votes on entire data buffers.
     * @param[out] output Buffer for corrected result
     * @param[in] buffer1 First input
     * @param[in] buffer2 Second input
     * @param[in] buffer3 Third input
     * @tparam T Type used for majority vote. Must be integral or enum type
     * @return True if all buffers are of the same size, False otherwise.
     */

    bool CorrectBuffer(gsl::span<std::uint8_t> output,
        gsl::span<const std::uint8_t> buffer1,
        gsl::span<const std::uint8_t> buffer2,
        gsl::span<const std::uint8_t> buffer3);

    /** @} */
}

#endif /* LIBS_BASE_INCLUDE_REDUNDANCY_HPP_ */
