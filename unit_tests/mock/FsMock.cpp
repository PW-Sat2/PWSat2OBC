#include "FsMock.hpp"
#include <utility>

static FSFileOpenResult FsOpen(FileSystem* fileSystem, const char* path, FSFileOpenFlags openFlag, FSFileAccessMode accessMode)
{
    auto fsMock = static_cast<FsMock*>(fileSystem);
    return fsMock->Open(path, openFlag, accessMode);
}

static OSResult FsTruncate(FileSystem* fileSystem, FSFileHandle file, FSFileSize length)
{
    auto fsMock = static_cast<FsMock*>(fileSystem);
    return fsMock->Truncate(file, length);
}

static FSIOResult FsWrite(FileSystem* fileSystem, FSFileHandle file, const void* buffer, FSFileSize size)
{
    auto fsMock = static_cast<FsMock*>(fileSystem);
    return fsMock->Write(file, buffer, size);
}

static FSIOResult FsRead(FileSystem* fileSystem, FSFileHandle file, void* buffer, FSFileSize size)
{
    auto fsMock = static_cast<FsMock*>(fileSystem);
    return fsMock->Read(file, buffer, size);
}

static OSResult FsClose(FileSystem* fileSystem, FSFileHandle file)
{
    auto fsMock = static_cast<FsMock*>(fileSystem);
    return fsMock->Close(file);
}

FsMock::FsMock()
{
    open = FsOpen;
    ftruncate = FsTruncate;
    write = FsWrite;
    read = FsRead;
    openDirectory = nullptr;
    readDirectory = nullptr;
    closeDirectory = nullptr;
    close = FsClose;
}

FSFileOpenResult MakeOpenedFile(int handle)
{
    FSFileOpenResult result;
    result.Status = OSResult::Success;
    result.Handle = handle;
    return result;
}

FSFileOpenResult MakeOpenedFile(OSResult status)
{
    FSFileOpenResult result;
    result.Status = status;
    result.Handle = -1;
    return result;
}

FSIOResult MakeFSIOResult(OSResult status)
{
    FSIOResult result;
    result.Status = status;
    result.BytesTransferred = 0;
    return result;
}

FSIOResult MakeFSIOResult(int bytesTransfered)
{
    FSIOResult result;
    result.Status = OSResult::Success;
    result.BytesTransferred = bytesTransfered;
    return result;
}
