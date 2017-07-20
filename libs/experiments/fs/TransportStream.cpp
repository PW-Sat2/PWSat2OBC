#include "TransportStream.hpp"

using namespace experiments::fs;

TransportStream::TransportStream(services::time::ICurrentTime* time) : _file(nullptr), _time(time), _writer(_buffer)
{
}

OSResult TransportStream::Open(services::fs::File* file)
{
    if (file == nullptr)
        return OSResult::DeviceNotFound;

    _file = file;
    InitializePacket();
    return OSResult::Success;
}

OSResult TransportStream::Write(PID pid, const gsl::span<uint8_t>& data)
{
    if (_file == nullptr)
        return OSResult::DeviceNotFound;

    auto dataSize = data.size();
    if (dataSize + PIDSize > static_cast<uint32_t>(_writer.RemainingSize()))
    {
        FillBufferWithPadding();
        auto result = Flush();
        if (OS_RESULT_FAILED(result))
            return result;
    }

    _writer.WriteByte(num(pid));
    _writer.WriteArray(data);

    return OSResult::Success;
}

OSResult TransportStream::Flush()
{
    if (_file == nullptr)
        return OSResult::DeviceNotFound;

    FillBufferWithPadding();
    auto result = _file->Write(_buffer);
    if (OS_RESULT_FAILED(result.Status))
        return result.Status;

    InitializePacket();

    return OSResult::Success;
}

void TransportStream::FillBufferWithPadding()
{
    auto freeSpace = _writer.RemainingSize();
    if (freeSpace < 1)
        return;

    _writer.WriteByte(num(PID::Padding));

    // starting from 1 as one byte has been written above
    for (int i = 1; i < freeSpace; ++i)
        _writer.WriteByte(0xFF);
}

void TransportStream::InitializePacket()
{
    _writer.Reset();
    _writer.WriteByte(num(PID::Synchronization));
    if (_time != nullptr)
    {
        const auto& timestamp = _time->GetCurrentTime();
        if (timestamp.HasValue)
        {
            _writer.WriteByte(num(PID::Timestamp));
            _writer.WriteQuadWordLE(_time->GetCurrentTime().Value.count());
        }
    }
}

OSResult TransportStream::Close()
{
    if (_file == nullptr)
        return OSResult::DeviceNotFound;

    FillBufferWithPadding();
    auto result = _file->Write(_buffer);

    if (OS_RESULT_FAILED(result.Status))
        return result.Status;

    return _file->Close();
}
