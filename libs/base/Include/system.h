#ifndef SRC_SYSTEM_H_
#define SRC_SYSTEM_H_

/**
 * @brief Macro for stating that routine parameter is not used.
 */
#define UNREFERENCED_PARAMETER(x) ((void)x)

/**
 * @brief Macro that calculates the number of elements in an array.
 */
#define COUNT_OF(x) (sizeof(x) / sizeof(*(x)))

#endif
