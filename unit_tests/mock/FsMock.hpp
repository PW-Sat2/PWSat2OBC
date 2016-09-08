#ifndef MOCK_FS_HPP
#define MOCK_FS_HPP

#pragma once

#include <string>
#include "gmock/gmock.h"
#include "fs/fs.h"

struct FsMock : FileSystem
{
    FsMock();
    MOCK_METHOD3(Open, FSFileOpenResult(const std::string& path, FSFileOpenFlags openFlag, FSFileAccessMode accessMode));
    MOCK_METHOD2(Truncate, OSResult(FSFileHandle file, FSFileSize length));
    MOCK_METHOD3(Write, FSIOResult(FSFileHandle file, const void* buffer, FSFileSize size));
    MOCK_METHOD3(Read, FSIOResult(FSFileHandle file, void* buffer, FSFileSize size));
    MOCK_METHOD1(Close, OSResult(FSFileHandle file));
};

FSFileOpenResult MakeOpenedFile(int handle);

FSFileOpenResult MakeOpenedFile(OSResult result);

FSIOResult MakeFSIOResult(int bytesTransfered);

FSIOResult MakeFSIOResult(OSResult result, int bytesTransfered);

#endif
