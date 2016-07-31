#include "FsMock.hpp"
#include <utility>

static FileSystem PrepareFileSystem();

FileSystem MockedFileSystem = PrepareFileSystem();

static FsMock* fsMock = nullptr;

FsMockReset::FsMockReset() : released(false)
{
}

FsMockReset::FsMockReset(FsMockReset&& arg) noexcept : released(arg.released)
{
    arg.released = true;
}

FsMockReset& FsMockReset::operator=(FsMockReset&& arg) noexcept
{
    FsMockReset tmp(std::move(arg));
    this->released = tmp.released;
    tmp.released = true;
    return *this;
}

FsMockReset::~FsMockReset()
{
    if (!released)
    {
        fsMock = nullptr;
    }
}

template <typename Pred, typename Result> Result CheckedCall(Pred pred, Result defaultValue)
{
    if (fsMock == nullptr)
    {
        return defaultValue;
    }
    else
    {
        return pred();
    }
}

static FSFileHandle FsOpen(const char* path, int openFlag, int mode)
{
    return CheckedCall([=]() { return fsMock->Open(path, openFlag, mode); }, -1);
}

static int FsTruncate(FSFileHandle file, int64_t length)
{
    return CheckedCall([=]() { return fsMock->Truncate(file, length); }, -1);
}

static int FsWrite(FSFileHandle file, const void* buffer, unsigned int size)
{
    return CheckedCall([=]() { return fsMock->Write(file, buffer, size); }, -1);
}

static int FsRead(FSFileHandle file, void* buffer, unsigned int size)
{
    return CheckedCall([=]() { return fsMock->Read(file, buffer, size); }, -1);
}

static int FsClose(FSFileHandle file)
{
    return CheckedCall([=]() { return fsMock->Close(file); }, -1);
}

static int FsGetLastError(void)
{
    return CheckedCall([=]() { return fsMock->GetLastError(); }, 0);
}

FsMockReset InstallFileSystemMock(FsMock& mock)
{
    fsMock = &mock;
    return FsMockReset();
}

FileSystem PrepareFileSystem()
{
    FileSystem mock;
    mock.open = FsOpen;
    mock.ftruncate = FsTruncate;
    mock.write = FsWrite;
    mock.read = FsRead;
    mock.openDirectory = nullptr;
    mock.readDirectory = nullptr;
    mock.closeDirectory = nullptr;
    mock.close = FsClose;
    mock.getLastError = FsGetLastError;
    return mock;
}
