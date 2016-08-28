#include "fs.h"
#include "logger/logger.h"

bool FileSystemSaveToFile(FileSystem* fs, const char* file, const uint8_t* buffer, FSFileSize length)
{
    const FSOpenResult result = fs->open(fs, file, FsOpenCreateAlways, FsWriteOnly);
    if (OS_RESULT_FAILED(result.Status))
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to open file: %s", file);
        return false;
    }

    const FSIOResult writeResult = fs->write(fs, result.FileHandle, buffer, length);
    const bool status = OS_RESULT_SUCCEEDED(writeResult.Status) && writeResult.BytesTransferred == length;
    if (!status)
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to update file: %s. Status: 0x%08x", file, writeResult.Status);
    }

    fs->close(fs, result.FileHandle);
    return status;
}

bool FileSystemReadFile(FileSystem* fs, const char* const filePath, uint8_t* buffer, FSFileSize length)
{
    const FSOpenResult result = fs->open(fs, filePath, FsOpenExisting, FsReadOnly);
    if (OS_RESULT_FAILED(result.Status))
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to open file: %s", filePath);
        return false;
    }

    const FSIOResult readResult = fs->read(fs, result.FileHandle, buffer, length);
    const bool status = OS_RESULT_SUCCEEDED(readResult.Status) && readResult.BytesTransferred == length;
    if (!status)
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to read file: %s. Status: 0x%08x", filePath, readResult.Status);
    }

    fs->close(fs, result.FileHandle);
    return status;
}
