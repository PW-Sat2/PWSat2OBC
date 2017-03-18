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
     * @brief Class used for TMR voting.
     * @tparam T Type used for majority vote. Must implement == operation.
     */
    template <typename T> struct Voter
    {
        /**
         * @brief Votes the actual value based on 3 provided inputs.
         * @param[in] a First input
         * @param[in] b Second input
         * @param[in] c Third input
         * @return Value based on majority vote, or None if all parameters are different.
         */
        static Option<T> Vote(T a, T b, T c)
        {
            if (a == b || a == c)
                return Some(a);
            if (b == c)
                return Some(b);

            return None<T>();
        }
    };

    /**
     * @brief Provides bitwise TMR.
     * @tparam T Type used for majority vote. Must implement bitwise & and | operations.
     */
    template <typename T> struct BitwiseCorrector
    {
        /**
         * @brief Performs bitwise majority vote based on three inputs.
         * @param[in] a First input
         * @param[in] b Second input
         * @param[in] c Third input
         * @return Value calculated from bitwise majority vote.
         */
        T Correct(T a, T b, T c)
        {
            return (a & b) | (b & c) | (a & c);
        }

        /**
         * @brief Performs bitwise majority votes on entire data buffers.
         * @param[in,out] buffer1 First input
         * @param[in] buffer2 Second input
         * @param[in] buffer3 Third input
         * @return True if all buffers are of the same size, False otherwise.
         */
        bool CorrectAll(gsl::span<T> buffer1, gsl::span<T> buffer2, gsl::span<T> buffer3)
        {
            if (buffer1.length() != buffer2.length() || buffer2.length() != buffer3.length())
                return false;

            for (auto i = 0; i < buffer1.length(); ++i)
            {
                buffer1[i] = Correct(buffer1[i], buffer2[i], buffer3[i]);
            }

            return true;
        }
    };

    /** @} */
}

#endif /* LIBS_BASE_INCLUDE_REDUNDANCY_HPP_ */
