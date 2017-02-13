/*
 * rtc.cpp
 *
 *  Created on: 08.02.2017
 *      Author: mdrobik
 */

#include "rtc.hpp"
#include "base/reader.h"
#include "logger/logger.h"

using namespace devices::rtc;
using drivers::i2c::II2CBus;
using drivers::i2c::I2CResult;

RTCObject::RTCObject(drivers::i2c::II2CBus& bus) : _bus(bus)
{
}

I2CResult RTCObject::ReadTime(RTCTime& rtcTime)
{
    std::array<std::uint8_t, 1> inBuffer;
    std::array<std::uint8_t, 7> outBuffer;

    inBuffer[0] = num(Registers::VL_seconds);

    const I2CResult result = _bus.WriteRead(I2CAddress, inBuffer, outBuffer);
    const bool status = (result == I2CResult::OK);
    if (!status)
    {
        LOG(LOG_LEVEL_ERROR, "Unable read time from RTC");
        return result;
    }

    Reader reader(outBuffer);

    rtcTime.seconds = reader.ReadByteBCD(SecondsNibbleMask);
    rtcTime.minutes = reader.ReadByteBCD(MinutesNibbleMask);
    rtcTime.hours = reader.ReadByteBCD(HoursNibbleMask);
    rtcTime.days = reader.ReadByteBCD(DaysNibbleMask);
    reader.Skip(1);
    rtcTime.months = reader.ReadByteBCD(MonthsNibbleMask);
    rtcTime.years = reader.ReadByteBCD(YearsNibbleMask);

    return I2CResult::OK;
}
