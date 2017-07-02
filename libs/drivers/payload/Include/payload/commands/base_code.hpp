#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_BASE_CODE_HPP_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_BASE_CODE_HPP_

#include "commands/base.h"
#include "logger/logger.h"

using namespace drivers::payload::commands;

template <std::uint8_t TCommandCode, class TOutputDataType>
PayloadCommand<TCommandCode, TOutputDataType>::PayloadCommand(IPayloadDriver& driver) : _driver(driver)
{
}

template <std::uint8_t TCommandCode, class TOutputDataType>
OSResult PayloadCommand<TCommandCode, TOutputDataType>::Execute(TOutputDataType& output)
{
    if (_driver.IsBusy())
    {
        LOG(LOG_LEVEL_WARNING, "Payload busy. Ignoring command");
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

template <std::uint8_t TCommandCode, class TOutputDataType>
OSResult PayloadCommand<TCommandCode, TOutputDataType>::ExecuteDataCommand(TOutputDataType& output)
{
    auto dataAddress = GetDataAddress();
    auto buffer = GetBuffer();

    auto result = ExecuteDataRead(dataAddress, buffer);
    if (result != OSResult::Success)
    {
        return result;
    }

    result = Save(buffer, output);
    if (result != OSResult::Success)
    {
        return result;
    }

    return OSResult::Success;
}

template <std::uint8_t TCommandCode, class TOutputDataType> OSResult PayloadCommand<TCommandCode, TOutputDataType>::ExecuteCommand()
{
    std::array<std::uint8_t, 1> commandBuffer = {CommandCode};

    auto result = _driver.PayloadWrite(commandBuffer);
    if (result != OSResult::Success)
    {
        LOGF(LOG_LEVEL_ERROR, "Unable to perform command 0x%x. Reason: %d", CommandCode, num(result));
        return result;
    }

    return OSResult::Success;
}

template <std::uint8_t TCommandCode, class TOutputDataType>
OSResult PayloadCommand<TCommandCode, TOutputDataType>::ExecuteDataRead(uint8_t address, gsl::span<uint8_t> buffer)
{
    std::array<std::uint8_t, 1> commandBuffer = {address};

    auto result = _driver.PayloadRead(commandBuffer, buffer);
    if (result != OSResult::Success)
    {
        LOGF(LOG_LEVEL_ERROR, "Unable to perform RadFET refresh. Reason: %d", num(result));
        return result;
    }

    return OSResult::Success;
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_BASE_CODE_HPP_ */
