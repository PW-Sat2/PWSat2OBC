#ifndef MOCK_FS_HPP
#define MOCK_FS_HPP

#pragma once

#include <string>
#include "gmock/gmock.h"
#include "fs/fs.h"

struct FsMock : FileSystem
{
    MOCK_METHOD3(open, FSFileOpenResult(const char* path, FSFileOpenFlags openFlag, FSFileAccessMode accessMode));
    MOCK_METHOD2(ftruncate, OSResult(FSFileHandle file, FSFileSize length));
    MOCK_METHOD3(write, FSIOResult(FSFileHandle file, const void* buffer, FSFileSize size));
    MOCK_METHOD3(read, FSIOResult(FSFileHandle file, void* buffer, FSFileSize size));
    MOCK_METHOD1(close, OSResult(FSFileHandle file));

    MOCK_METHOD1(openDirectory, FSDirectoryOpenResult(const char*));
    MOCK_METHOD1(readDirectory, char*(FSDirectoryHandle));
    MOCK_METHOD1(closeDirectory, OSResult(FSDirectoryHandle));
    MOCK_METHOD1(format, OSResult(const char*));
    MOCK_METHOD1(makeDirectory, OSResult(const char*));
    MOCK_METHOD1(exists, bool(const char*));
    MOCK_METHOD1(ClearDevice, OSResult(yaffs_dev*));
    MOCK_METHOD0(Sync, void());
};

FSFileOpenResult MakeOpenedFile(int handle);

FSFileOpenResult MakeOpenedFile(OSResult result);

FSIOResult MakeFSIOResult(int bytesTransfered);

FSIOResult MakeFSIOResult(OSResult result);

FSIOResult MakeFSIOResult(OSResult result, int bytesTransfered);

#endif
