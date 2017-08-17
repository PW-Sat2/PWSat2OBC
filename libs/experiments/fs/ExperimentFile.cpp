#include "ExperimentFile.hpp"
#include <utility>

using namespace experiments::fs;
using namespace services::fs;

ExperimentFile::ExperimentFile(services::time::ICurrentTime* time)
    : _time(time), _writer(_buffer), _hasPayloadInFrame(false),
      onFlush(OnFlushDelegate::make_delegate<ExperimentFile, &ExperimentFile::DoNothing>(this))
{
    _buffer.fill(0xAA);
}

ExperimentFile::~ExperimentFile()
{
    if (_hasPayloadInFrame)
    {
        Close();
    }
}

ExperimentFile::ExperimentFile(ExperimentFile&& other)
    : _buffer(other._buffer), _time(other._time), _writer(_buffer), _hasPayloadInFrame(other._hasPayloadInFrame),
      onFlush(OnFlushDelegate::make_delegate<ExperimentFile, &ExperimentFile::DoNothing>(this))
{
    other._time = nullptr;
    other._hasPayloadInFrame = 0;
    _file = std::move(other._file);
    _writer.Reserve(other._writer.GetDataLength());
}

ExperimentFile& ExperimentFile::operator=(ExperimentFile&& other)
{
    ExperimentFile tmp(std::move(other));

    std::swap(_buffer, tmp._buffer);
    std::swap(_file, tmp._file);
    std::swap(_time, tmp._time);
    std::swap(_writer, tmp._writer);
    std::swap(_hasPayloadInFrame, tmp._hasPayloadInFrame);

    return *this;
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
        }

        WriteDataBiggerThanFrame(pid, data);
    }

    return OSResult::Success;
}

OSResult ExperimentFile::WriteDataBiggerThanFrame(PID pid, const gsl::span<uint8_t>& data)
{
    uint32_t offset = 0;
    uint32_t dataRemaining = data.size();
    while (dataRemaining > 0)
    {
        const auto length = std::min(dataRemaining, static_cast<uint32_t>(_writer.RemainingSize()) - PIDSize);

        auto subpart = data.subspan(offset, length);
        _writer.WriteByte(num(pid));
        _writer.WriteArray(subpart);
        _hasPayloadInFrame = true;

        if (_writer.RemainingSize() == 0)
        {
            auto result = Flush();
            if (OS_RESULT_FAILED(result))
            {
                return result;
            }

            _writer.WriteByte(num(PID::Continuation));
        }

        offset += length;
        dataRemaining -= length;
    }

    return OSResult::Success;
}

OSResult ExperimentFile::Flush()
{
    return FlushInternal(true);
}

OSResult ExperimentFile::FlushInternal(bool initialize)
{
    FillBufferWithPadding();
    auto result = _file.Write(_buffer);
    if (OS_RESULT_FAILED(result.Status))
    {
        return result.Status;
    }

    onFlush(gsl::make_span(_buffer));

    if (initialize)
    {
        InitializePacket();
    }

    return OSResult::Success;
}

void ExperimentFile::SetOnFlush(OnFlushDelegate onFlush)
{
    this->onFlush = onFlush;
}

void ExperimentFile::DoNothing(const gsl::span<uint8_t>&)
{
}

void ExperimentFile::FillBufferWithPadding()
{
    if (_writer.RemainingSize() < 1)
    {
        return;
    }

    _writer.WriteByte(num(PID::Padding));
    _writer.Fill(PaddingData);
    _hasPayloadInFrame = true;
}

void ExperimentFile::InitializePacket()
{
    _writer.Reset();
    _writer.WriteByte(num(PID::Synchronization));
    if (_time != nullptr)
    {
        const auto timestamp = _time->GetCurrentTime();
        if (timestamp.HasValue)
        {
            _writer.WriteByte(num(PID::Timestamp));
            _writer.WriteQuadWordLE(timestamp.Value.count());
        }
    }

    _hasPayloadInFrame = false;
}

OSResult ExperimentFile::Close()
{
    if (_hasPayloadInFrame)
    {
        auto result = FlushInternal(false);
        if (OS_RESULT_FAILED(result))
        {
            return result;
        }
    }

    return _file.Close();
}
