#ifndef MOCK_FS_HPP
#define MOCK_FS_HPP

#pragma once

#include <string>
#include "gmock/gmock.h"
#include "fs/fs.h"

struct FsMock
{
    MOCK_METHOD3(Open, FSFileHandle(const std::string& path, int flags, int mode));
    MOCK_METHOD2(Truncate, int(FSFileHandle file, int64_t length));
    MOCK_METHOD3(Write, int(FSFileHandle file, const void* buffer, unsigned int size));
    MOCK_METHOD3(Read, int(FSFileHandle file, void* buffer, unsigned int size));
    MOCK_METHOD1(Close, int(FSFileHandle file));
    MOCK_METHOD0(GetLastError, int(void));
};

class FsMockReset
{
  public:
    FsMockReset();

    FsMockReset(FsMockReset&& arg) noexcept;

    ~FsMockReset();

    FsMockReset& operator=(FsMockReset&& arg) noexcept;

  private:
    bool released;
};

extern FileSystem MockedFileSystem;

FsMockReset InstallFileSystemMock(FsMock& mock);

#endif
