#include <algorithm>
#include "fs.h"
#include "logger/logger.h"
#include "system.h"

using namespace services::fs;

bool services::fs::SaveToFile(IFileSystem& fs, const char* file, gsl::span<const std::uint8_t> buffer)
{
    File f(fs, file, FileOpen::CreateAlways, FileAccess::WriteOnly);

    if (!f)
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to open file: %s", file);
        return false;
    }

    const IOResult writeResult = f.Write(buffer);
    const bool status = OS_RESULT_SUCCEEDED(writeResult.Status) && writeResult.Result.size() == buffer.size();
    if (!status)
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to update file: %s. Status: 0x%08x", file, num(writeResult.Status));
    }

    return status;
}

bool services::fs::ReadFromFile(IFileSystem& fs, const char* const filePath, gsl::span<std::uint8_t> buffer)
{
    File f(fs, filePath, FileOpen::Existing, FileAccess::ReadOnly);
    if (!f)
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to open file: %s", filePath);
        return false;
    }

    const IOResult readResult = f.Read(buffer);
    const bool status = OS_RESULT_SUCCEEDED(readResult.Status) && readResult.Result.size() == buffer.size();
    if (!status)
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to read file: %s. Status: 0x%08x", filePath, num(readResult.Status));
    }

    return status;
}

File::~File()
{
    if (*this)
    {
        this->_fs->Close(this->_handle);
    }
}

File::File() : _fs(nullptr)
{
}

File::File(IFileSystem& fs, const char* path, FileOpen mode, FileAccess access) : _fs(&fs)
{
    auto f = fs.Open(path, mode, access);
    this->_handle = f.Result;
    this->_valid = f;
}

IOResult File::Read(gsl::span<uint8_t> buffer)
{
    if (!*this)
    {
        return IOResult(OSResult::InvalidOperation, gsl::span<const uint8_t>());
    }

    return this->_fs->Read(this->_handle, buffer);
}

IOResult File::Write(gsl::span<const uint8_t> buffer)
{
    if (!*this)
    {
        return IOResult(OSResult::InvalidOperation, gsl::span<const uint8_t>());
    }

    return this->_fs->Write(this->_handle, buffer);
}

OSResult File::Truncate(FileSize size)
{
    if (!*this)
    {
        return OSResult::InvalidOperation;
    }

    return this->_fs->TruncateFile(this->_handle, size);
}

FileSize File::Size()
{
    if (!*this)
    {
        return 0;
    }

    return this->_fs->GetFileSize(this->_handle);
}

File& File::operator=(File&& other)
{
    std::swap(this->_fs, other._fs);
    std::swap(this->_handle, other._handle);
    std::swap(this->_valid, other._valid);

    return *this;
}

OSResult File::Close()
{
    if (*this)
    {
        this->_fs->Close(this->_handle);
        this->_fs = nullptr;
        this->_valid = false;
        this->_handle = 0;

        return OSResult::Success;
    }

    return OSResult::InvalidOperation;
}
