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

// struct MissionClock
//{
//    typedef std::chrono::miliseconds          duration;
//    typedef duration::rep                     rep;
//    typedef duration::period                  period;
//    typedef std::chrono::time_point<MissionClock> time_point;
//    static const bool is_steady =             false;
//
//    static time_point now() noexcept
//    {
//        using namespace std::chrono;
//        return time_point
//          (
//
//          );
//    }
//};

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
 * @brief Type used to measure current mission time in milliseconds.
 *
 * The point with zero time (beginning of time) is considered to be
 * beginning of the mission itself.
 */
typedef std::chrono::duration<uint64_t, std::milli> TimeSpan;

/**
 * @brief Type used to represents different between two time spans.
 */
typedef std::chrono::duration<uint64_t, std::milli> TimeShift;

/**
 * @brief Creates TimeSpan object initialized from time specified in milliseconds.
 * @param[in] milliseconds TimeSpan value in milliseconds.
 * @return Prepared time span object.
 */
TimeSpan TimeSpanFromMilliseconds(uint64_t milliseconds);

/**
 * @brief Creates TimeSpan object initialized from time specified in seconds.
 * @param[in] seconds TimeSpan value in seconds.
 * @return Prepared time span object.
 */
TimeSpan TimeSpanFromSeconds(uint32_t seconds);

/**
 * @brief Creates TimeSpan object initialized from time specified in minutes.
 * @param[in] minutes TimeSpan value in minutes.
 * @return Prepared time span object.
 */
TimeSpan TimeSpanFromMinutes(uint32_t minutes);

/**
 * @brief Creates TimeSpan object initialized from time specified in hours.
 * @param[in] hours TimeSpan value in hours.
 * @return Prepared time span object.
 */
TimeSpan TimeSpanFromHours(uint32_t hours);

/**
 * @brief Creates TimeSpan object initialized from time specified in days.
 * @param[in] days TimeSpan value in days.
 * @return Prepared time span object.
 */
TimeSpan TimeSpanFromDays(uint32_t days);

/**
 * @brief Adds together two TimeSpan objects.
 * @param[in] left TimeSpan value to add.
 * @param[in] right TimeSpan value to add.
 * @return Addition result.
 */
TimeSpan TimeSpanAdd(TimeSpan left, TimeSpan right);

/**
 * @brief Subtracts two TimeSpan objects.
 * @param[in] left TimeSpan value to subtract from.
 * @param[in] right TimeSpan value to be subtracted.
 * @return Subtraction result.
 */
TimeShift TimeSpanSub(TimeSpan left, TimeSpan right);

/**
 * @brief Compares two TimeSpan objects.
 * @param[in] left TimeSpan value to compare.
 * @param[in] right TimeSpan value to compare.
 * @return Comparison result. True when two TimeSpans are equal, false otherwise.
 */
static bool TimeSpanEqual(const TimeSpan left, const TimeSpan right);

/**
 * @brief Compares two TimeSpan objects.
 * @param[in] left TimeSpan value to compare.
 * @param[in] right TimeSpan value to compare.
 * @return Comparison result. True when two TimeSpans are not equal, false otherwise.
 */
static bool TimeSpanNotEqual(const TimeSpan left, const TimeSpan right);

/**
 * @brief Determines whether left TimeSpan object is strictly smaller than right object.
 * @param[in] left TimeSpan value to compare.
 * @param[in] right TimeSpan value to compare.
 * @return Comparison result. True when left TimeSpan object is strictly smaller than right object, false otherwise.
 */
static bool TimeSpanLessThan(const TimeSpan left, const TimeSpan right);

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
TimePoint TimePointFromTimeSpan(TimeSpan span);

/**
 * @brief This procedure converts decoded time point object into time span value.
 * @param[in] point Time point object to convert.
 * @return Converted time span value.
 */
TimeSpan TimePointToTimeSpan(TimePoint point);

/**
 * @brief This procedure converts passed timespan value to time span that is
 * measured in seconds.
 * @param[in] span Time span to convert.
 * @return Time span in seconds.
 */
uint32_t TimeSpanToSeconds(TimeSpan span);

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
    return TimeSpanEqual(TimePointToTimeSpan(left), TimePointToTimeSpan(right));
}

static inline bool TimePointLessThan(TimePoint left, TimePoint right)
{
    return TimeSpanLessThan(TimePointToTimeSpan(left), TimePointToTimeSpan(right));
}

static inline bool TimeSpanEqual(const TimeSpan left, const TimeSpan right)
{
    return left == right;
}

static inline bool TimeSpanNotEqual(const TimeSpan left, const TimeSpan right)
{
    return left != right;
}

static inline bool TimeSpanLessThan(const TimeSpan left, const TimeSpan right)
{
    return left < right;
}

/** @} */

EXTERNC_END

#endif
