/*
 * rtc.cpp
 *
 *  Created on: 08.02.2017
 *      Author: mdrobik
 */

#include "rtc.hpp"
#include "logger/logger.h"

using namespace devices::rtc;
using drivers::i2c::II2CBus;
using drivers::i2c::I2CResult;

RTCObject::RTCObject(drivers::i2c::II2CBus& bus) : _bus(bus)
{
}

void RTCObject::ReadTime(RTCTime& rtcTime)
{
    std::array<std::uint8_t, 1> inBuffer;
    std::array<std::uint8_t, 7> outBuffer;

    inBuffer[0] = num(Registers::VL_seconds);

    const I2CResult result = _bus.WriteRead(I2CAddress, inBuffer, outBuffer);
    const bool status = (result == I2CResult::OK);
    if (!status)
    {
        LOG(LOG_LEVEL_ERROR, "Unable read time from RTC");
        return;
    }

    rtcTime.seconds = ConvertFromBCD(outBuffer[0], SecondsNibbleMask);
    rtcTime.minutes = ConvertFromBCD(outBuffer[1], MinutesNibbleMask);
    rtcTime.hours = ConvertFromBCD(outBuffer[2], HoursNibbleMask);
    rtcTime.days = ConvertFromBCD(outBuffer[3], DaysNibbleMask);
    rtcTime.months = ConvertFromBCD(outBuffer[5], MonthsNibbleMask);
    rtcTime.years = ConvertFromBCD(outBuffer[6], YearsNibbleMask);
}

std::uint8_t RTCObject::ConvertFromBCD(std::uint8_t bcd, std::uint8_t upperNibbleMask)
{
    return ((bcd & upperNibbleMask) >> 4) * 10 + (bcd & 0x0F);
}
