/*
 * rtc.cpp
 *
 *  Created on: 08.02.2017
 *      Author: mdrobik
 */

#include "rtc.hpp"
#include "base/reader.h"
#include "base/writer.h"
#include "logger/logger.h"

using namespace devices::rtc;
using drivers::i2c::II2CBus;
using drivers::i2c::I2CResult;

RTCObject::RTCObject(drivers::i2c::II2CBus& bus) : _bus(bus)
{
}

OSResult RTCObject::ReadTime(RTCTime& rtcTime)
{
    std::array<std::uint8_t, 1> inBuffer;
    std::array<std::uint8_t, 7> outBuffer;

    inBuffer[0] = num(Registers::VL_seconds);

    const I2CResult result = _bus.WriteRead(I2CAddress, inBuffer, outBuffer);
    const bool status = (result == I2CResult::OK);
    if (!status)
    {
        LOGF(LOG_LEVEL_ERROR, "Unable read time from RTC. Reason: %d", num(result));
        return OSResult::InvalidOperation;
    }

    Reader reader(outBuffer);

    rtcTime.seconds = reader.ReadByteBCD(SecondsNibbleMask);
    rtcTime.minutes = reader.ReadByteBCD(MinutesNibbleMask);
    rtcTime.hours = reader.ReadByteBCD(HoursNibbleMask);
    rtcTime.days = reader.ReadByteBCD(DaysNibbleMask);
    reader.Skip(1);
    rtcTime.months = reader.ReadByteBCD(MonthsNibbleMask);
    rtcTime.years = reader.ReadByteBCD(YearsNibbleMask);
    return OSResult::Success;
}

bool RTCObject::IsIntegrityGuaranteed()
{
    std::array<std::uint8_t, 1> inBuffer;
    std::array<std::uint8_t, 1> outBuffer;

    inBuffer[0] = num(Registers::VL_seconds);

    const I2CResult result = _bus.WriteRead(I2CAddress, inBuffer, outBuffer);
    const bool status = (result == I2CResult::OK);
    if (!status)
    {
        LOGF(LOG_LEVEL_ERROR, "Unable read integrity flag from RTC. Reason: %d", num(result));
        return false;
    }

    LOGF(LOG_LEVEL_INFO, "[rtc] Integrity flag: 0x%X", outBuffer[0]);

    return !has_flag(outBuffer[0], 1 << 7);
}

OSResult RTCObject::SetTime(const RTCTime& time)
{
    std::array<std::uint8_t, 8> inBuffer;
    Writer w(inBuffer);
    w.WriteByte(num(Registers::VL_seconds));
    w.WriteByteBCD(time.seconds);
    w.WriteByteBCD(time.minutes);
    w.WriteByteBCD(time.hours);
    w.WriteByteBCD(time.days);
    w.WriteByteBCD(0);
    w.WriteByteBCD(time.months);
    w.WriteByteBCD(time.years);

    const I2CResult result = _bus.Write(I2CAddress, inBuffer);
    const bool status = (result == I2CResult::OK);
    if (!status)
    {
        LOGF(LOG_LEVEL_ERROR, "Unable read integrity flag from RTC. Reason: %d", num(result));
        return OSResult::DeviceNotFound;
    }

    return OSResult::Success;
}

OSResult RTCObject::Initialize()
{
    auto isClockOk = this->IsIntegrityGuaranteed();

    if (isClockOk)
    {
        return OSResult::Success;
    }

    RTCTime zero;
    zero.years = 0;
    zero.months = 1;
    zero.days = 1;
    zero.hours = 0;
    zero.minutes = 0;
    zero.seconds = 0;

    LOG(LOG_LEVEL_WARNING, "[rtc] Integrity not guaranteed. Reseting RTC to 0");
    return this->SetTime(zero);
}
