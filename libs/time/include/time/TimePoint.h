#ifndef SRC_TIME_POINT_H
#define SRC_TIME_POINT_H

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <chrono>
#include "system.h"

EXTERNC_BEGIN

/**
 * @addtogroup time
 * @{
 */

/**
 * @brief Structure that contains decoded point in time.
 *
 * The fields of this type does not have any ranges applied to them,
 * therefore object with i.e. 200 minutes is still valid. Such object
 * is considered to be non canonical state and may be normalized to
 * one that has 3 hours and 20 minutes instead of 200 minutes.
 */
typedef struct
{
    uint8_t second;      ///< Number of seconds.
    uint8_t minute;      ///< Number of minutes.
    uint8_t hour;        ///< Number of hours
    uint16_t day;        ///< Number of days.
    uint16_t milisecond; ///< Number of milliseconds.
} TimePoint;

/**
 * @brief Creates TimeSpan object initialized from time specified in days.
 * @param[in] days TimeSpan value in days.
 * @return Prepared time span object.
 */
std::chrono::milliseconds TimeSpanFromDays(uint32_t days);

/**
 * @brief This procedure builds Time point object.
 *
 * @param[in] day Number of days.
 * @param[in] hour Number of hours.
 * @param[in] minute Number of minutes.
 * @param[in] second Number of seconds.
 * @param[in] millisecond Number of milliseconds.
 *
 * @return Build TimePoint object.
 */
TimePoint TimePointBuild(uint16_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t millisecond);

/**
 * @brief This procedure converts passed time span value into normalized decoded time point object.
 * @param[in] span Time span to convert.
 * @return Decoded time point object.
 */
TimePoint TimePointFromDuration(std::chrono::milliseconds span);

/**
 * @brief This procedure converts decoded time point object into time span value.
 * @param[in] point Time point object to convert.
 * @return Converted time span value.
 */
std::chrono::milliseconds TimePointToTimeSpan(TimePoint point);

/**
 * @brief This procedure normalizes TimePoint object.
 *
 * Normalization here means ensuring that all TimePoint members contain values that fit into
 * their intended ranges:
 *  - 0 - 999 for milliseconds
 *  - 0 - 59 for seconds
 *  - 0 - 59 for minutes
 *  - 0 - 23 for hours
 *  - any value for days.
 *
 *  @param[in] point TimePoint value for normalization.
 *  @return Normalized decoded TimePoint value.
 */
TimePoint TimePointNormalize(TimePoint point);

/**
 * This procedure checks whether two time point values are considered to be equal.
 *
 * This procedure is capable of comparing normalized with non normalized values.
 * @param[in] left Time Point value for comparison.
 * @param[in] right Time Point value for comparison.
 * @return True if both values are considered to be equal, false otherwise.
 */
static bool TimePointEqual(TimePoint left, TimePoint right);

/**
 * @brief This procedure compares two TimePoint values and determines
 * whether first argument is considered to be less then the second one.
 * @param[in] left Time Point value for comparison.
 * @param[in] right Time Point value for comparison.
 * @return True if left argument is considered to be less than right argument, false otherwise.
 */
static bool TimePointLessThan(TimePoint left, TimePoint right);

/**
 * This procedure checks whether two time point values are considered to be not equal.
 *
 * This procedure is capable of comparing normalized with non normalized values.
 * @param[in] left Time Point value for comparison.
 * @param[in] right Time Point value for comparison.
 * @return True if both values are not considered to be equal, false otherwise.
 */
static inline bool TimePointNotEqual(TimePoint left, TimePoint right)
{
    return !TimePointEqual(left, right);
}

static inline bool TimePointEqual(TimePoint left, TimePoint right)
{
    return TimePointToTimeSpan(left) == TimePointToTimeSpan(right);
}

static inline bool TimePointLessThan(TimePoint left, TimePoint right)
{
    return TimePointToTimeSpan(left) < TimePointToTimeSpan(right);
}

/** @} */

EXTERNC_END

#endif
