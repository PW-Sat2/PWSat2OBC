#include <algorithm>
#include <array>
#include <cstring>

#include "base/os.h"

#include "n25q.h"

using std::uint8_t;
using std::min;
using std::ptrdiff_t;
using gsl::span;

using namespace devices::n25q;
using redundancy::Vote;
using redundancy::CorrectBuffer;

RedundantN25QDriver::RedundantN25QDriver(std::array<IN25QDriver*, 3> n25qDrivers) : _n25qDrivers(n25qDrivers)
{
}

void RedundantN25QDriver::ReadMemory(    //
    std::size_t address,                 //
    gsl::span<uint8_t> outputBuffer,     //
    gsl::span<uint8_t> redundantBuffer1, //
    gsl::span<uint8_t> redundantBuffer2)
{
    auto bufferLength = std::min(outputBuffer.length(), std::min(redundantBuffer1.length(), redundantBuffer2.length()));

    auto normalizedOutputBuffer = outputBuffer.subspan(0, bufferLength);
    auto normalizedRedundantBuffer1 = redundantBuffer1.subspan(0, bufferLength);

    _n25qDrivers[0]->ReadMemory(address, normalizedOutputBuffer);
    _n25qDrivers[1]->ReadMemory(address, normalizedRedundantBuffer1);

    auto compareResult = memcmp(normalizedOutputBuffer.data(), normalizedRedundantBuffer1.data(), normalizedOutputBuffer.size()) == 0;

    if (compareResult)
    {
        return;
    }

    {
        auto normalizedRedundantBuffer2 = redundantBuffer2.subspan(0, bufferLength);

        _n25qDrivers[2]->ReadMemory(address, normalizedRedundantBuffer2);
        CorrectBuffer(normalizedOutputBuffer, normalizedRedundantBuffer1, normalizedRedundantBuffer2);
    }
}

OperationResult RedundantN25QDriver::EraseChip()
{
    auto d1Wait = _n25qDrivers[0]->BeginEraseChip();
    auto d2Wait = _n25qDrivers[1]->BeginEraseChip();
    auto d3Wait = _n25qDrivers[2]->BeginEraseChip();

    auto d1Result = d1Wait.Wait();
    auto d2Result = d2Wait.Wait();
    auto d3Result = d3Wait.Wait();

    auto votedResult = Vote(d1Result, d2Result, d3Result);

    if (!votedResult.HasValue)
        return OperationResult::Failure;

    return votedResult.Value;
}

OperationResult RedundantN25QDriver::EraseSubSector(size_t address)
{
    auto d1Wait = _n25qDrivers[0]->BeginEraseSubSector(address);
    auto d2Wait = _n25qDrivers[1]->BeginEraseSubSector(address);
    auto d3Wait = _n25qDrivers[2]->BeginEraseSubSector(address);

    auto d1Result = d1Wait.Wait();
    auto d2Result = d2Wait.Wait();
    auto d3Result = d3Wait.Wait();

    auto votedResult = Vote(d1Result, d2Result, d3Result);

    if (!votedResult.HasValue)
        return OperationResult::Failure;

    return votedResult.Value;
}

OperationResult RedundantN25QDriver::EraseSector(size_t address)
{
    auto d1Wait = _n25qDrivers[0]->BeginEraseSector(address);
    auto d2Wait = _n25qDrivers[1]->BeginEraseSector(address);
    auto d3Wait = _n25qDrivers[2]->BeginEraseSector(address);

    auto d1Result = d1Wait.Wait();
    auto d2Result = d2Wait.Wait();
    auto d3Result = d3Wait.Wait();

    auto votedResult = Vote(d1Result, d2Result, d3Result);

    if (!votedResult.HasValue)
        return OperationResult::Failure;

    return votedResult.Value;
}

OperationResult RedundantN25QDriver::WriteMemory(size_t address, gsl::span<const uint8_t> buffer)
{
    for (ptrdiff_t offset = 0; offset < buffer.size(); offset += N25QDriver::PageSize)
    {
        auto page = buffer.subspan(offset, min(N25QDriver::PageSize, buffer.size() - offset));

        auto d1Waiter = _n25qDrivers[0]->BeginWritePage(address, offset, page);
        auto d2Waiter = _n25qDrivers[1]->BeginWritePage(address, offset, page);
        auto d3Waiter = _n25qDrivers[2]->BeginWritePage(address, offset, page);

        auto d1Result = d1Waiter.Wait();
        auto d2Result = d2Waiter.Wait();
        auto d3Result = d3Waiter.Wait();

        auto votedResult = Vote(d1Result, d2Result, d3Result);
        if (!votedResult.HasValue)
            return OperationResult::Failure;
        if (votedResult.Value != OperationResult::Success)
            return votedResult.Value;
    }

    return OperationResult::Success;
}

OperationResult RedundantN25QDriver::Reset()
{
    auto d1Result = _n25qDrivers[0]->Reset();
    auto d2Result = _n25qDrivers[1]->Reset();
    auto d3Result = _n25qDrivers[2]->Reset();

    auto votedResult = Vote(d1Result, d2Result, d3Result);
    if (!votedResult.HasValue)
        return OperationResult::Failure;
    if (votedResult.Value != OperationResult::Success)
        return votedResult.Value;

    return OperationResult::Success;
}
