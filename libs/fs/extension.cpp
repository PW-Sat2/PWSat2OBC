#include "fs.h"
#include "logger/logger.h"
#include "system.h"

bool FileSystemSaveToFile(FileSystem& fs, const char* file, gsl::span<const std::uint8_t> buffer)
{
    const FSFileOpenResult result = fs.Open(file, FSFileOpen::CreateAlways, FSFileAccess::WriteOnly);
    if (OS_RESULT_FAILED(result.Status))
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to open file: %s", file);
        return false;
    }

    const FSIOResult writeResult = fs.Write(result.Handle, buffer);
    const bool status = OS_RESULT_SUCCEEDED(writeResult.Status) && writeResult.BytesTransferred == buffer.size();
    if (!status)
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to update file: %s. Status: 0x%08x", file, num(writeResult.Status));
    }

    fs.Close(result.Handle);
    return status;
}

bool FileSystemReadFile(FileSystem& fs, const char* const filePath, gsl::span<std::uint8_t> buffer)
{
    const FSFileOpenResult result = fs.Open(filePath, FSFileOpen::Existing, FSFileAccess::ReadOnly);
    if (OS_RESULT_FAILED(result.Status))
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to open file: %s", filePath);
        return false;
    }

    const FSIOResult readResult = fs.Read(result.Handle, buffer);
    const bool status = OS_RESULT_SUCCEEDED(readResult.Status) && readResult.BytesTransferred == buffer.size();
    if (!status)
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to read file: %s. Status: 0x%08x", filePath, num(readResult.Status));
    }

    fs.Close(result.Handle);
    return status;
}
