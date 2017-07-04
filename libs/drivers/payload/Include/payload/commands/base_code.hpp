#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_BASE_CODE_HPP_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_BASE_CODE_HPP_

#include "commands/base.h"
#include "logger/logger.h"

using namespace drivers::payload::commands;

template <std::uint8_t TCommandCode, class TOutputDataType, const uint8_t TDeviceDataAddress, const uint8_t TDeviceDataLength>
PayloadCommand<TCommandCode, TOutputDataType, TDeviceDataAddress, TDeviceDataLength>::PayloadCommand(IPayloadDriver& driver)
    : _driver(driver)
{
    _buffer.fill(0xFF);
}

template <std::uint8_t TCommandCode, class TOutputDataType, const uint8_t TDeviceDataAddress, const uint8_t TDeviceDataLength>
OSResult PayloadCommand<TCommandCode, TOutputDataType, TDeviceDataAddress, TDeviceDataLength>::Execute(TOutputDataType& output)
{
    if (_driver.IsBusy())
    {
        LOG(LOG_LEVEL_WARNING, "[Payload] Payload busy. Ignoring command");
        return OSResult::Busy;
    }

    OSResult result = ExecuteCommand();
    if (result != OSResult::Success)
    {
        return result;
    }

    result = _driver.WaitForData();
    if (result != OSResult::Success)
    {
        return result;
    }

    return ExecuteDataCommand(output);
}

template <std::uint8_t TCommandCode, class TOutputDataType, const uint8_t TDeviceDataAddress, const uint8_t TDeviceDataLength>
OSResult PayloadCommand<TCommandCode, TOutputDataType, TDeviceDataAddress, TDeviceDataLength>::ExecuteDataCommand(TOutputDataType& output)
{
    auto result = ExecuteDataRead(DeviceDataAddress, _buffer);
    if (result != OSResult::Success)
    {
        return result;
    }

    result = Save(_buffer, output);
    if (result != OSResult::Success)
    {
        return result;
    }

    return OSResult::Success;
}

template <std::uint8_t TCommandCode, class TOutputDataType, const uint8_t TDeviceDataAddress, const uint8_t TDeviceDataLength>
OSResult PayloadCommand<TCommandCode, TOutputDataType, TDeviceDataAddress, TDeviceDataLength>::ExecuteCommand()
{
    std::array<std::uint8_t, 1> commandBuffer = {CommandCode};

    auto result = _driver.PayloadWrite(commandBuffer);
    if (result != OSResult::Success)
    {
        LOGF(LOG_LEVEL_ERROR, "[Payload] Unable to perform command 0x%x. Reason: %d", CommandCode, num(result));
        return result;
    }

    return OSResult::Success;
}

template <std::uint8_t TCommandCode, class TOutputDataType, const uint8_t TDeviceDataAddress, const uint8_t TDeviceDataLength>
OSResult PayloadCommand<TCommandCode, TOutputDataType, TDeviceDataAddress, TDeviceDataLength>::ExecuteDataRead(
    uint8_t address, gsl::span<uint8_t> buffer)
{
    std::array<std::uint8_t, 1> commandBuffer = {address};

    auto result = _driver.PayloadRead(commandBuffer, buffer);
    if (result != OSResult::Success)
    {
        LOGF(LOG_LEVEL_ERROR, "[Payload] Unable to perform RadFET refresh. Reason: %d", num(result));
        return result;
    }

    return OSResult::Success;
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_BASE_CODE_HPP_ */
