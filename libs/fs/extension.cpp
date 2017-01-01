#include "fs.h"
#include "logger/logger.h"
#include "system.h"

bool FileSystemSaveToFile(FileSystem& fs, const char* file, gsl::span<const std::uint8_t> buffer)
{
    auto f = File::Open(fs, file, FSFileOpen::CreateAlways, FSFileAccess::WriteOnly);

    if (!f)
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to open file: %s", file);
        return false;
    }

    const FSIOResult writeResult = f.Write(buffer);
    const bool status = OS_RESULT_SUCCEEDED(writeResult.Status) && writeResult.BytesTransferred == buffer.size();
    if (!status)
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to update file: %s. Status: 0x%08x", file, num(writeResult.Status));
    }

    return status;
}

bool FileSystemReadFile(FileSystem& fs, const char* const filePath, gsl::span<std::uint8_t> buffer)
{
    auto f = File::OpenRead(fs, filePath);
    if (!f)
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to open file: %s", filePath);
        return false;
    }

    const FSIOResult readResult = f.Read(buffer);
    const bool status = OS_RESULT_SUCCEEDED(readResult.Status) && readResult.BytesTransferred == buffer.size();
    if (!status)
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to read file: %s. Status: 0x%08x", filePath, num(readResult.Status));
    }

    return status;
}

File::File(FileSystem& fs, FSFileOpenResult open) : _fs(fs), _handle(open.Handle), _valid(OS_RESULT_SUCCEEDED(open.Status))
{
}

File::~File()
{
    if (this->_valid)
    {
        this->_fs.Close(this->_handle);
    }
}

File::File(File&& other) noexcept : _fs(other._fs), _handle(other._handle), _valid(other._valid)
{
    other._handle = this->_handle;
    other._valid = this->_valid;
    other._fs = this->_fs;
}

File& File::operator=(File&& other) noexcept
{
    this->_handle = other._handle;
    this->_valid = other._valid;
    this->_fs = other._fs;

    other._handle = this->_handle;
    other._valid = this->_valid;
    other._fs = this->_fs;

    return *this;
}

File File::Open(FileSystem& fs, const char* path, FSFileOpen mode, FSFileAccess access)
{
    auto f = fs.Open(path, mode, access);

    return File(fs, f);
}

File File::OpenRead(FileSystem& fs, const char* path, FSFileOpen mode, FSFileAccess access)
{
    return Open(fs, path, mode, access);
}

File File::OpenWrite(FileSystem& fs, const char* path, FSFileOpen mode, FSFileAccess access)
{
    return Open(fs, path, mode, access);
}

FSIOResult File::Read(gsl::span<uint8_t> buffer)
{
    return this->_fs.Read(this->_handle, buffer);
}

FSIOResult File::Write(gsl::span<const uint8_t> buffer)
{
    return this->_fs.Write(this->_handle, buffer);
}

OSResult File::Truncate(FSFileSize size)
{
    return this->_fs.TruncateFile(this->_handle, size);
}
