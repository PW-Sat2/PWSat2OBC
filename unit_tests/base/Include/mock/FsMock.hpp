#ifndef MOCK_FS_HPP
#define MOCK_FS_HPP

#pragma once

#include <map>
#include <string>
#include "gmock/gmock.h"
#include "fs/fs.h"

struct OpenedFile
{
    std::string File;

    gsl::span<std::uint8_t>::iterator Position;
};

struct OpenedDir
{
    std::string path;
    std::map<std::string, gsl::span<std::uint8_t>>::iterator Position;
};

class FsMock : public services::fs::IFileSystem
{
  public:
    FsMock();
    ~FsMock();

    MOCK_METHOD3(
        Open, services::fs::FileOpenResult(const char* path, services::fs::FileOpen openFlag, services::fs::FileAccess accessMode));
    MOCK_METHOD1(Unlink, OSResult(const char* path));
    MOCK_METHOD2(Move, OSResult(const char* from, const char* to));
    MOCK_METHOD2(Copy, OSResult(const char* from, const char* to));
    MOCK_METHOD2(TruncateFile, OSResult(services::fs::FileHandle file, services::fs::FileSize length));
    MOCK_METHOD2(Write, services::fs::IOResult(services::fs::FileHandle file, gsl::span<const std::uint8_t> buffer));
    MOCK_METHOD2(Read, services::fs::IOResult(services::fs::FileHandle file, gsl::span<std::uint8_t> buffer));
    MOCK_METHOD1(Close, OSResult(services::fs::FileHandle file));

    MOCK_METHOD1(OpenDirectory, services::fs::DirectoryOpenResult(const char*));
    MOCK_METHOD1(ReadDirectory, char*(services::fs::DirectoryHandle));
    MOCK_METHOD1(CloseDirectory, OSResult(services::fs::DirectoryHandle));
    MOCK_METHOD1(IsDirectory, bool(const char*));
    MOCK_METHOD1(Format, OSResult(const char*));
    MOCK_METHOD1(MakeDirectory, OSResult(const char*));
    MOCK_METHOD1(Exists, bool(const char*));
    MOCK_METHOD1(GetFileSize, services::fs::FileSize(services::fs::FileHandle));
    MOCK_METHOD2(GetFileSize, services::fs::FileSize(const char* dir, const char* file));
    MOCK_METHOD3(Seek, OSResult(services::fs::FileHandle file, services::fs::SeekOrigin origin, services::fs::FileSize offset));
    MOCK_METHOD1(GetFreeSpace, std::uint32_t(const char* devicePath));

    void AddFile(const char* path, gsl::span<std::uint8_t> contents);

  private:
    std::map<std::string, gsl::span<std::uint8_t>> _files;
    services::fs::FileHandle _nextHandle;
    std::map<services::fs::FileHandle, OpenedFile> _opened;

    std::uint32_t _nextDirHandle;
    std::map<services::fs::DirectoryHandle, OpenedDir> _openedDirs;
};

services::fs::FileOpenResult MakeOpenedFile(int handle);

services::fs::FileOpenResult MakeOpenedFile(OSResult result);

services::fs::IOResult MakeFSIOResult(gsl::span<const uint8_t> result);

services::fs::IOResult MakeFSIOResult(OSResult result);

#endif
