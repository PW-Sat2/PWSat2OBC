#include "fs.h"
#include "logger/logger.h"

bool FileSystemSaveToFile(FileSystem* fs, const char* file, const uint8_t* buffer, uint32_t length)
{
    const FSFileHandle handle = fs->open(file, O_WRONLY | O_CREAT, S_IRWXU);
    if (handle == -1)
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to open file: %s", file);
        return 0;
    }

    const uint32_t result = fs->write(handle, buffer, length);
    const bool status = result == length;
    if (!status)
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to update file: %s. Status: 0x%08x", file, fs->getLastError());
    }

    fs->close(handle);
    return status;
}

bool FileSystemReadFile(FileSystem* fs, const char* const filePath, uint8_t* buffer, uint32_t length)
{
    const FSFileHandle handle = fs->open(filePath, O_RDONLY, S_IRWXU);
    if (handle == -1)
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to open file: %s", filePath);
        return false;
    }

    const uint32_t read = fs->read(handle, buffer, length);
    const bool status = read == length;
    if (!status)
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to read file: %s. Status: 0x%08x", filePath, fs->getLastError());
    }

    fs->close(handle);
    return status;
}
