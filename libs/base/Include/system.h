#ifndef SRC_SYSTEM_H_
#define SRC_SYSTEM_H_

#include <stddef.h>

/**
 * @brief Macro for stating that routine parameter is not used.
 */
#define UNREFERENCED_PARAMETER(x) ((void)x)

#define UNUSED1(a) ((void)a)
#define UNUSED2(a, b) ((void)a), UNUSED1(b)
#define UNUSED3(a, b, c) ((void)a), UNUSED2(b, c)
#define UNUSED4(a, b, c, d) ((void)a), UNUSED3(b, c, d)
#define UNUSED5(a, b, c, d, e) ((void)a), UNUSED4(b, c, d, e)

#define VA_NUM_ARGS_IMPL(_1, _2, _3, _4, _5, N, ...) N
#define VA_NUM_ARGS(...) VA_NUM_ARGS_IMPL(__VA_ARGS__, 5, 4, 3, 2, 1)

#define UNUSED_IMPL_(nargs) UNUSED##nargs
#define UNUSED_IMPL(nargs) UNUSED_IMPL_(nargs)
#define UNUSED(...) UNUSED_IMPL(VA_NUM_ARGS(__VA_ARGS__))(__VA_ARGS__)

/**
 * @brief Macro that calculates the number of elements in an array.
 */
#define COUNT_OF(x) (sizeof(x) / sizeof(*(x)))

/**
 * @brief Tells GCC to support unaligned access
 */
#ifndef __packed
#define __packed __attribute__((aligned(1)))
#endif

#ifdef __cplusplus
#define EXTERNC_BEGIN extern "C" {
#define EXTERNC_END }
#else
#define EXTERNC_BEGIN
#define EXTERNC_END
#endif

#define HAS_FLAG(value, flag) (((value) & (flag)) == (flag))

#define IS_BIT_SET(value, bit) (((value) & (1u << bit)) == (1 << bit))

#define IS_BIT_CLEAR(value, bit) (((value) & (1u << bit)) == 0)

#endif
