#include "ExperimentFile.hpp"

using namespace experiments::fs;
using namespace services::fs;

ExperimentFile::ExperimentFile(services::time::ICurrentTime* time) : _time(time), _writer(_buffer), _hasPayloadInFrame(false)
{
}

bool ExperimentFile::Open(IFileSystem& fs, const char* path, FileOpen mode, FileAccess access)
{
    _file = File(fs, path, mode, access);

    if (!_file)
        return false;

    InitializePacket();
    return true;
}

OSResult ExperimentFile::Write(PID pid, const gsl::span<uint8_t>& data)
{
    auto dataSize = data.size();

    if (dataSize + PIDSize <= static_cast<uint32_t>(_writer.RemainingSize()))
    {
        // data will fit the frame.
        _writer.WriteByte(num(pid));
        _writer.WriteArray(data);
        _hasPayloadInFrame = true;
    }
    else
    {
        if (_hasPayloadInFrame)
        {
            // frame free space has ended. Restart frame.
            FillBufferWithPadding();
            auto result = Flush();
            if (OS_RESULT_FAILED(result))
            {
                return result;
            }

            _writer.WriteByte(num(pid));
            _writer.WriteArray(data);
            _hasPayloadInFrame = true;
        }
        else
        {
            // data bigger than frame. Split data by several frames.
            uint32_t offset = 0;
            uint32_t length = _writer.RemainingSize() - PIDSize;
            while (static_cast<uint32_t>(data.length()) > offset + length)
            {
                auto&& subpart = data.subspan(offset, length);
                _writer.WriteByte(num(pid));
                _writer.WriteArray(subpart);

                FillBufferWithPadding();
                auto result = Flush();
                if (OS_RESULT_FAILED(result))
                {
                    return result;
                }

                offset += length;
            }
        }
    }

    return OSResult::Success;
}

OSResult ExperimentFile::Flush()
{
    FillBufferWithPadding();
    auto result = _file.Write(_buffer);
    if (OS_RESULT_FAILED(result.Status))
    {
        return result.Status;
    }

    InitializePacket();

    return OSResult::Success;
}

void ExperimentFile::FillBufferWithPadding()
{
    auto freeSpace = _writer.RemainingSize();
    if (freeSpace < 1)
    {
        return;
    }

    _writer.WriteByte(num(PID::Padding));

    // starting from 1 as one byte has been written above
    for (int i = 1; i < freeSpace; ++i)
    {
        _writer.WriteByte(0xFF);
    }

    _hasPayloadInFrame = true;
}

void ExperimentFile::InitializePacket()
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

    _hasPayloadInFrame = false;
}

OSResult ExperimentFile::Close()
{
    FillBufferWithPadding();
    auto result = _file.Write(_buffer);

    if (OS_RESULT_FAILED(result.Status))
    {
        return result.Status;
    }

    return _file.Close();
}
