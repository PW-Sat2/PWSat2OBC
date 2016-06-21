#include <stdio.h>
#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include "system.h"
#include "yaffs_guts.h"
#include "yaffsfs.h"

#include "MemoryDriver.hpp"

using testing::StrEq;
using testing::Eq;
using testing::Test;

extern "C" void yaffs_remove_device(struct yaffs_dev* dev);

class FileSystemTest : public Test
{
  protected:
    yaffs_dev device;

    int32_t FillDevice(int file);

  public:
    FileSystemTest();
    ~FileSystemTest();
};

FileSystemTest::FileSystemTest()
{
    memset(&device, 0, sizeof(device));
    device.param.name = "/";

    InitializeYaffsDev(&device);

    yaffs_add_device(&device);
}

FileSystemTest::~FileSystemTest()
{
    yaffs_remove_device(&device);
}

int32_t FileSystemTest::FillDevice(int file)
{
    uint8_t buffer[1024];
    for (uint16_t i = 0; i < COUNT_OF(buffer); i++)
    {
        buffer[i] = i % 256;
    }

    int32_t totalSize = 0;

    for (uint16_t i = 0; i < 1023; i++)
    {
        int32_t written = yaffs_write(file, buffer, COUNT_OF(buffer));

        totalSize += written;

        if (written < COUNT_OF(buffer))
        {
            break;
        }
    }

    return totalSize;
}

TEST_F(FileSystemTest, SanityCheck)
{
    yaffs_mount("/");

    auto file = yaffs_open("/file", O_CREAT | O_WRONLY, S_IRWXU);

    char buffer[40] = "Hello World";

    yaffs_write(file, buffer, strlen(buffer));

    yaffs_close(file);

    file = yaffs_open("/file", O_RDONLY, S_IRWXU);

    memset(buffer, 0, COUNT_OF(buffer));

    yaffs_read(file, buffer, COUNT_OF(buffer));

    yaffs_close(file);

    yaffs_unmount("/");

    yaffs_mount("/");

    yaffs_unmount("/");

    ASSERT_THAT(buffer, StrEq("Hello World"));
}

TEST_F(FileSystemTest, WritingFileBiggerThatOneChunk)
{
    yaffs_mount("/");

    auto file = yaffs_open("/file", O_CREAT | O_WRONLY, S_IRWXU);

    uint8_t buffer[1024];
    for (uint16_t i = 0; i < COUNT_OF(buffer); i++)
    {
        buffer[i] = i % 256;
    }

    yaffs_write(file, buffer, COUNT_OF(buffer));

    yaffs_close(file);

    yaffs_unmount("/");

    yaffs_mount("/");

    file = yaffs_open("/file", O_RDONLY, S_IRWXU);

    memset(buffer, 0, COUNT_OF(buffer));

    yaffs_read(file, buffer, COUNT_OF(buffer));

    yaffs_close(file);

    yaffs_unmount("/");

    for (uint16_t i = 0; i < COUNT_OF(buffer); i++)
    {
        ASSERT_THAT(buffer[i], Eq(i % 256));
    }
}

TEST_F(FileSystemTest, OverwritingWholeDeviceWithSingleFile)
{
    yaffs_mount("/");

    auto file = yaffs_open("/file", O_CREAT | O_WRONLY, S_IRWXU);

    auto totalSize = FillDevice(file);

    yaffs_close(file);

    yaffs_unmount("/");

    yaffs_mount("/");

    file = yaffs_open("/file", O_RDONLY, S_IRWXU);

    struct yaffs_stat stats;
    memset(&stats, 0, sizeof(stats));

    yaffs_fstat(file, &stats);

    ASSERT_THAT(stats.st_size, Eq(totalSize));

    yaffs_close(file);

    yaffs_unmount("/");
}

TEST_F(FileSystemTest, BlocksFailingWhileWriting)
{
    yaffs_mount("/");

    CauseBadBlock(&device, 1);

    auto file = yaffs_open("/file", O_CREAT | O_WRONLY, S_IRWXU);

    FillDevice(file);

    yaffs_close(file);

    yaffs_format("/", 1, 1, 0);

    ASSERT_THAT(IsBadBlock(&device, 1), Eq(true));
}
