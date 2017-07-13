#ifndef BASE_HERTZ_HPP
#define BASE_HERTZ_HPP

#include <chrono>

namespace chrono_extensions
{
    template <typename _Rep, typename _Period = std::ratio<1>> struct frequency;

    template <typename _Tp> struct __is_frequency : std::false_type
    {
    };

    template <typename _Rep, typename _Period> struct __is_frequency<frequency<_Rep, _Period>> : std::true_type
    {
    };

    template <typename _ToFreq, typename _CF, typename _CR, bool _NumIsOne = false, bool _DenIsOne = false> struct __frequency_cast_impl
    {
        template <typename _Rep, typename _Period> static constexpr _ToFreq __cast(const frequency<_Rep, _Period>& __d)
        {
            typedef typename _ToFreq::rep __to_rep;
            return _ToFreq(static_cast<__to_rep>(static_cast<_CR>(__d.count()) * static_cast<_CR>(_CF::num) / static_cast<_CR>(_CF::den)));
        }
    };

    template <typename _ToFreq, typename _CF, typename _CR> struct __frequency_cast_impl<_ToFreq, _CF, _CR, true, true>
    {
        template <typename _Rep, typename _Period> static constexpr _ToFreq __cast(const frequency<_Rep, _Period>& __d)
        {
            typedef typename _ToFreq::rep __to_rep;
            return _ToFreq(static_cast<__to_rep>(__d.count()));
        }
    };

    template <typename _ToFreq, typename _CF, typename _CR> struct __frequency_cast_impl<_ToFreq, _CF, _CR, true, false>
    {
        template <typename _Rep, typename _Period> static constexpr _ToFreq __cast(const frequency<_Rep, _Period>& __d)
        {
            typedef typename _ToFreq::rep __to_rep;
            return _ToFreq(static_cast<__to_rep>(static_cast<_CR>(__d.count()) / static_cast<_CR>(_CF::den)));
        }
    };

    template <typename _ToFreq, typename _CF, typename _CR> struct __frequency_cast_impl<_ToFreq, _CF, _CR, false, true>
    {
        template <typename _Rep, typename _Period> static constexpr _ToFreq __cast(const frequency<_Rep, _Period>& __d)
        {
            typedef typename _ToFreq::rep __to_rep;
            return _ToFreq(static_cast<__to_rep>(static_cast<_CR>(__d.count()) * static_cast<_CR>(_CF::num)));
        }
    };

    template <typename _ToFreq, typename _Rep, typename _Period>
    constexpr typename std::enable_if<__is_frequency<_ToFreq>::value, _ToFreq>::type frequency_cast(const frequency<_Rep, _Period>& __d)
    {
        typedef typename _ToFreq::period __to_period;
        typedef typename _ToFreq::rep __to_rep;
        typedef std::ratio_divide<_Period, __to_period> __cf;
        typedef typename std::common_type<__to_rep, _Rep, intmax_t>::type __cr;
        typedef __frequency_cast_impl<_ToFreq, __cf, __cr, __cf::num == 1, __cf::den == 1> __dc;
        return __dc::__cast(__d);
    }

    template <typename _Rep> struct frequency_values
    {
        static constexpr _Rep zero()
        {
            return _Rep(0);
        }

        static constexpr _Rep max()
        {
            return std::numeric_limits<_Rep>::max();
        }

        static constexpr _Rep min()
        {
            return std::numeric_limits<_Rep>::lowest();
        }
    };

    template <typename _Rep, typename _Period> struct frequency
    {
        typedef _Rep rep;
        typedef _Period period;

        static_assert(!std::chrono::__is_duration<_Rep>::value, "rep cannot be a duration");
        static_assert(std::chrono::__is_ratio<_Period>::value, "period must be a specialization of ratio");
        static_assert(_Period::num > 0, "period must be positive");

        constexpr frequency() = default;

        frequency(const frequency&) = default;

        template <typename _Rep2,
            typename = typename std::enable_if<std::is_convertible<_Rep2, rep>::value &&
                (std::chrono::treat_as_floating_point<rep>::value || !std::chrono::treat_as_floating_point<_Rep2>::value)>::type>
        constexpr explicit frequency(const _Rep2& __rep) : __r(static_cast<rep>(__rep))
        {
        }

        template <typename _Rep2,
            typename _Period2,
            typename = typename std::enable_if<std::chrono::treat_as_floating_point<rep>::value ||
                (std::ratio_divide<_Period2, period>::den == 1 && !std::chrono::treat_as_floating_point<_Rep2>::value)>::type>
        constexpr frequency(const frequency<_Rep2, _Period2>& __d) : __r(frequency_cast<frequency>(__d).count())
        {
        }

        ~frequency() = default;
        frequency& operator=(const frequency&) = default;

        constexpr rep count() const
        {
            return __r;
        }

        constexpr frequency operator+() const
        {
            return *this;
        }

        constexpr frequency operator-() const
        {
            return frequency(-__r);
        }

        frequency& operator++()
        {
            ++__r;
            return *this;
        }

        frequency operator++(int)
        {
            return frequency(__r++);
        }

        frequency& operator--()
        {
            --__r;
            return *this;
        }

        frequency operator--(int)
        {
            return frequency(__r--);
        }

        frequency& operator+=(const frequency& __d)
        {
            __r += __d.count();
            return *this;
        }

        frequency& operator-=(const frequency& __d)
        {
            __r -= __d.count();
            return *this;
        }

        frequency& operator*=(const rep& __rhs)
        {
            __r *= __rhs;
            return *this;
        }

        frequency& operator/=(const rep& __rhs)
        {
            __r /= __rhs;
            return *this;
        }

        static constexpr frequency zero()
        {
            return frequency(frequency_values<rep>::zero());
        }

        static constexpr frequency min()
        {
            return frequency(frequency_values<rep>::min());
        }

        static constexpr frequency max()
        {
            return frequency(frequency_values<rep>::max());
        }

      private:
        rep __r;
    };

    template <typename _Rep1, typename _Period1, typename _Rep2, typename _Period2>
    constexpr bool operator==(const frequency<_Rep1, _Period1>& __lhs, const frequency<_Rep2, _Period2>& __rhs)
    {
        typedef frequency<_Rep1, _Period1> __dur1;
        typedef frequency<_Rep2, _Period2> __dur2;
        typedef typename std::common_type<__dur1, __dur2>::type __ct;
        return __ct(__lhs).count() == __ct(__rhs).count();
    }

    template <typename _Rep1, typename _Period1, typename _Rep2, typename _Period2>
    constexpr bool operator<(const std::chrono::duration<_Rep1, _Period1>& __lhs, const frequency<_Rep2, _Period2>& __rhs)
    {
        typedef std::chrono::duration<_Rep1, _Period1> __dur1;
        typedef std::chrono::duration<_Rep2, _Period2> __dur2;
        typedef typename std::common_type<__dur1, __dur2>::type __ct;
        return __ct(__lhs).count() < __ct(__rhs).count();
    }

    template <typename _Rep1, typename _Period1, typename _Rep2, typename _Period2>
    constexpr bool operator!=(const frequency<_Rep1, _Period1>& __lhs, const frequency<_Rep2, _Period2>& __rhs)
    {
        return !(__lhs == __rhs);
    }

    template <typename _Rep1, typename _Period1, typename _Rep2, typename _Period2>
    constexpr bool operator<=(const frequency<_Rep1, _Period1>& __lhs, const frequency<_Rep2, _Period2>& __rhs)
    {
        return !(__rhs < __lhs);
    }

    template <typename _Rep1, typename _Period1, typename _Rep2, typename _Period2>
    constexpr bool operator>(const frequency<_Rep1, _Period1>& __lhs, const frequency<_Rep2, _Period2>& __rhs)
    {
        return __rhs < __lhs;
    }

    template <typename _Rep1, typename _Period1, typename _Rep2, typename _Period2>
    constexpr bool operator>=(const frequency<_Rep1, _Period1>& __lhs, const frequency<_Rep2, _Period2>& __rhs)
    {
        return !(__lhs < __rhs);
    }

    template <typename _ToDur, typename _CF, typename _CR, bool _NumIsOne = false, bool _DenIsOne = false> struct __period_cast_impl
    {
        template <typename _Rep, typename _Period> static constexpr _ToDur __cast(const frequency<_Rep, _Period>& __d)
        {
            typedef typename _ToDur::rep __to_rep;
            return _ToDur(
                static_cast<__to_rep>(1.0 / static_cast<_CR>(__d.count()) * static_cast<_CR>(_CF::num) / static_cast<_CR>(_CF::den)));
        }
    };

    template <typename _ToDur, typename _CF, typename _CR> struct __period_cast_impl<_ToDur, _CF, _CR, true, true>
    {
        template <typename _Rep, typename _Period> static constexpr _ToDur __cast(const frequency<_Rep, _Period>& __d)
        {
            typedef typename _ToDur::rep __to_rep;
            return _ToDur(static_cast<__to_rep>(1.0 / __d.count()));
        }
    };

    template <typename _ToDur, typename _CF, typename _CR> struct __period_cast_impl<_ToDur, _CF, _CR, true, false>
    {
        template <typename _Rep, typename _Period> static constexpr _ToDur __cast(const frequency<_Rep, _Period>& __d)
        {
            typedef typename _ToDur::rep __to_rep;
            return _ToDur(static_cast<__to_rep>(1.0 / static_cast<_CR>(__d.count()) / static_cast<_CR>(_CF::den)));
        }
    };

    template <typename _ToDur, typename _CF, typename _CR> struct __period_cast_impl<_ToDur, _CF, _CR, false, true>
    {
        template <typename _Rep, typename _Period> static constexpr _ToDur __cast(const frequency<_Rep, _Period>& __d)
        {
            typedef typename _ToDur::rep __to_rep;
            return _ToDur(static_cast<__to_rep>(1.0 / static_cast<_CR>(__d.count()) * static_cast<_CR>(_CF::num)));
        }
    };

    template <typename _ToDur, typename _Rep, typename _Period>
    constexpr typename std::enable_if<std::chrono::__is_duration<_ToDur>::value, _ToDur>::type period_cast(
        const frequency<_Rep, _Period>& __d)
    {
        typedef typename _ToDur::period __to_period;
        typedef typename _ToDur::rep __to_rep;
        typedef std::ratio_divide<_Period, __to_period> __cf;
        typedef typename std::common_type<__to_rep, _Rep, intmax_t>::type __cr;
        typedef __period_cast_impl<_ToDur, __cf, __cr, __cf::num == 1, __cf::den == 1> __dc;
        return __dc::__cast(__d);
    }

    typedef frequency<long double> hertz;

    namespace chrono_literals
    {
        constexpr chrono_extensions::hertz operator"" _hz(long double hz)
        {
            return chrono_extensions::hertz(hz);
        }

        constexpr chrono_extensions::hertz operator"" _hz(unsigned long long hz)
        {
            return chrono_extensions::hertz(hz);
        }
    }
}

#endif /* BASE_HERTZ_HPP */
