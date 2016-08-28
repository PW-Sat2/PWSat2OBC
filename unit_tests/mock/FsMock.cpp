#include "FsMock.hpp"
#include <utility>

static FSOpenResult FsOpen(FileSystem* fileSystem, const char* path, FSFileOpenFlags openFlag, FSFileAccessMode accessMode)
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

static int FsGetLastError(FileSystem* fileSystem)
{
    auto fsMock = static_cast<FsMock*>(fileSystem);
    return fsMock->GetLastError();
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
    getLastError = FsGetLastError;
}

FSOpenResult MakeOpenedFile(int handle)
{
    FSOpenResult result;
    result.Status = OSResultSuccess;
    result.FileHandle = handle;
    return result;
}

FSOpenResult MakeOpenedFile(OSResult status)
{
    FSOpenResult result;
    result.Status = status;
    result.FileHandle = -1;
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
    result.Status = OSResultSuccess;
    result.BytesTransferred = bytesTransfered;
    return result;
}
