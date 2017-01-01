#ifndef MOCK_FS_HPP
#define MOCK_FS_HPP

#pragma once

#include <string>
#include "gmock/gmock.h"
#include "fs/fs.h"

struct FsMock : FileSystem
{
    MOCK_METHOD3(Open, FSFileOpenResult(const char* path, FSFileOpen openFlag, FSFileAccess accessMode));
    MOCK_METHOD2(TruncateFile, OSResult(FSFileHandle file, FSFileSize length));
    MOCK_METHOD2(Write, FSIOResult(FSFileHandle file, gsl::span<const std::uint8_t> buffer));
    MOCK_METHOD2(Read, FSIOResult(FSFileHandle file, gsl::span<std::uint8_t> buffer));
    MOCK_METHOD1(Close, OSResult(FSFileHandle file));

    MOCK_METHOD1(OpenDirectory, FSDirectoryOpenResult(const char*));
    MOCK_METHOD1(ReadDirectory, char*(FSDirectoryHandle));
    MOCK_METHOD1(CloseDirectory, OSResult(FSDirectoryHandle));
    MOCK_METHOD1(Format, OSResult(const char*));
    MOCK_METHOD1(MakeDirectory, OSResult(const char*));
    MOCK_METHOD1(Exists, bool(const char*));
    MOCK_METHOD1(ClearDevice, OSResult(yaffs_dev*));
    MOCK_METHOD0(Sync, void());
};

FSFileOpenResult MakeOpenedFile(int handle);

FSFileOpenResult MakeOpenedFile(OSResult result);

FSIOResult MakeFSIOResult(int bytesTransfered);

FSIOResult MakeFSIOResult(OSResult result);

FSIOResult MakeFSIOResult(OSResult result, int bytesTransfered);

#endif
