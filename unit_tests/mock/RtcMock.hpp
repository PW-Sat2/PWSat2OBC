#ifndef UNIT_TESTS_RTC_RTCMOCK_HPP_
#define UNIT_TESTS_RTC_RTCMOCK_HPP_

#pragma once

#include "gmock/gmock.h"
#include "rtc/rtc.hpp"

class RtcMock : public devices::rtc::RTCObject
{
  public:
    I2CBusMock i2cMock;

    RtcMock() : devices::rtc::RTCObject(i2cMock), readTimeResult(OSResult::Success)
    {
        fakeTime.seconds = 0;
        fakeTime.minutes = 0;
        fakeTime.hours = 0;
        fakeTime.days = 1;
        fakeTime.months = 1;
        fakeTime.years = 0;
    }

    OSResult ReadTime(devices::rtc::RTCTime& rtcTime) override
    {
        rtcTime = fakeTime;

        return readTimeResult;
    }

    void SetTime(std::chrono::seconds duration)
    {
        auto timePoint = TimePointNormalize(TimePointFromDuration(duration));

        fakeTime.seconds = timePoint.second;
        fakeTime.minutes = timePoint.minute;
        fakeTime.hours = timePoint.hour;
    }

    void AdvanceTime(std::chrono::seconds delta)
    {
        SetTime(fakeTime.ToDuration() + delta);
    }

    void SetReadResult(OSResult result)
    {
        readTimeResult = result;
    }

  private:
    devices::rtc::RTCTime fakeTime;
    OSResult readTimeResult;
};

#endif /* UNIT_TESTS_RTC_RTCMOCK_HPP_ */
