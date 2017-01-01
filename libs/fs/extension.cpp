#include "fs.h"
#include "logger/logger.h"
#include "system.h"

bool FileSystemSaveToFile(FileSystem* fs, const char* file, const uint8_t* buffer, FSFileSize length)
{
    const FSFileOpenResult result = fs->open(file, FsOpenCreateAlways, FsWriteOnly);
    if (OS_RESULT_FAILED(result.Status))
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to open file: %s", file);
        return false;
    }

    const FSIOResult writeResult = fs->write(result.Handle, buffer, length);
    const bool status = OS_RESULT_SUCCEEDED(writeResult.Status) && writeResult.BytesTransferred == length;
    if (!status)
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to update file: %s. Status: 0x%08x", file, num(writeResult.Status));
    }

    fs->close(result.Handle);
    return status;
}

bool FileSystemReadFile(FileSystem* fs, const char* const filePath, uint8_t* buffer, FSFileSize length)
{
    const FSFileOpenResult result = fs->open(filePath, FsOpenExisting, FsReadOnly);
    if (OS_RESULT_FAILED(result.Status))
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to open file: %s", filePath);
        return false;
    }

    const FSIOResult readResult = fs->read(result.Handle, buffer, length);
    const bool status = OS_RESULT_SUCCEEDED(readResult.Status) && readResult.BytesTransferred == length;
    if (!status)
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to read file: %s. Status: 0x%08x", filePath, num(readResult.Status));
    }

    fs->close(result.Handle);
    return status;
}
