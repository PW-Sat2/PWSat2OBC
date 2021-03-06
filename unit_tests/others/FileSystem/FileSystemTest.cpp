#include <stdio.h>
#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "fs/yaffs.h"
#include "yaffs.hpp"

#include "FileSystem/MemoryDriver.hpp"

#include "storage/nand_driver.h"
#include "system.h"

using testing::StrEq;
using testing::Eq;
using testing::Ne;
using testing::Ge;
using testing::Test;
using namespace services::fs;

extern "C" void yaffs_remove_device(struct yaffs_dev* dev);
namespace
{
    class FileSystemTest : public Test
    {
      protected:
        yaffs_dev device;
        YaffsNANDDriver driver;
        YaffsFileSystem api;

        int32_t FillDevice(int file);

      public:
        FileSystemTest();
        ~FileSystemTest();
    };

    FileSystemTest::FileSystemTest()
    {
        memset(&driver, 0, sizeof(driver));
        driver.geometry.pageSize = 512;
        driver.geometry.spareAreaPerPage = 12; // 16 - bad block mark - 3 bytes ECC
        driver.geometry.pagesPerBlock = 32;
        driver.geometry.pagesPerChunk = 2;

        NANDCalculateGeometry(&driver.geometry);

        InitializeMemoryNAND(&driver.flash);

        memset(&device, 0, sizeof(device));

        SetupYaffsNANDDriver(&device, &driver);

        device.param.name = "/";
        device.param.inband_tags = false;
        device.param.is_yaffs2 = true;
        device.param.total_bytes_per_chunk = driver.geometry.chunkSize;
        device.param.chunks_per_block = driver.geometry.chunksPerBlock;
        device.param.spare_bytes_per_chunk = driver.geometry.spareAreaPerPage * driver.geometry.pagesPerChunk;
        device.param.start_block = 1;
        device.param.n_reserved_blocks = 3;
        device.param.no_tags_ecc = true;
        device.param.always_check_erased = true;

        device.param.end_block = 1 * 1024 * 1024 / driver.geometry.blockSize - device.param.start_block - device.param.n_reserved_blocks;

        yaffs_add_device(&device);
    }

    FileSystemTest::~FileSystemTest()
    {
        yaffs_remove_device(&device);
    }

    int32_t FileSystemTest::FillDevice(int file)
    {
        const size_t bufferSize = 10240;
        auto buffer = new uint8_t[bufferSize];
        for (uint16_t i = 0; i < bufferSize; i++)
        {
            buffer[i] = (uint8_t)(i & 0xFF);
        }

        int32_t totalSize = 0;

        for (uint16_t i = 0;; i++)
        {
            int32_t written = yaffs_write(file, buffer, bufferSize);

            totalSize += written;

            if (written < (int32_t)bufferSize)
            {
                break;
            }
        }

        delete[] buffer;

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

        yaffs_unlink("/file");

        yaffs_unmount("/");

        yaffs_mount("/");

        yaffs_unmount("/");

        ASSERT_THAT(buffer, StrEq("Hello World"));
    }

    TEST_F(FileSystemTest, WritingFileBiggerThatOneChunk)
    {
        yaffs_mount("/");

        auto file = yaffs_open("/file", O_CREAT | O_WRONLY, S_IRWXU);

        uint8_t buffer[2048];
        for (uint16_t i = 0; i < COUNT_OF(buffer); i++)
        {
            buffer[i] = (uint8_t)(i & 0xFF);
        }

        yaffs_write(file, buffer, COUNT_OF(buffer));

        yaffs_close(file);

        yaffs_unmount("/");

        yaffs_mount("/");

        file = yaffs_open("/file", O_RDONLY, S_IRWXU);

        memset(buffer, 0, sizeof(buffer));

        yaffs_read(file, buffer, sizeof(buffer));

        yaffs_close(file);

        yaffs_unmount("/");

        for (uint16_t i = 0; i < COUNT_OF(buffer); i++)
        {
            ASSERT_THAT(buffer[i], Eq((uint8_t)(i & 0xFF)));
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

        CauseBadBlock(&driver.flash, 1);

        auto file = yaffs_open("/file", O_CREAT | O_WRONLY, S_IRWXU);

        FillDevice(file);

        yaffs_close(file);
        printf("Format\n");
        yaffs_format("/", 1, 1, 0);

        ASSERT_THAT(IsBadBlock(&driver.flash, 1), Eq(true));
    }

    TEST_F(FileSystemTest, ShouldCorrectSingleBitError)
    {
        uint8_t buffer[1024];
        for (uint16_t i = 0; i < COUNT_OF(buffer); i++)
        {
            buffer[i] = (uint8_t)(i & 0xFF);
        }

        yaffs_mount("/");

        auto file = yaffs_open("/file", O_CREAT | O_WRONLY, S_IRWXU);

        yaffs_write(file, buffer, sizeof(buffer));

        auto obj = yaffs_get_obj_from_fd(file);

        auto nand_chunk = yaffs_find_chunk_in_file(obj, 1, NULL);

        yaffs_close(file);

        yaffs_unmount("/");

        SwapBit(&driver.flash, nand_chunk * driver.geometry.chunkSize + 100, 1);

        yaffs_flush_whole_cache(&device, 1);

        yaffs_mount("/");

        file = yaffs_open("/file", O_RDONLY, S_IRWXU);

        memset(buffer, 0, sizeof(buffer));

        yaffs_read(file, buffer, sizeof(buffer));

        for (uint16_t i = 0; i < COUNT_OF(buffer); i++)
        {
            ASSERT_THAT(buffer[i], Eq((uint8_t)(i & 0xFF)));
        }

        yaffs_close(file);

        yaffs_unmount("/");
    }

    TEST_F(FileSystemTest, ShouldDetectUncorrectableError)
    {
        uint8_t buffer[1024];
        for (uint16_t i = 0; i < COUNT_OF(buffer); i++)
        {
            buffer[i] = i % 256;
        }

        yaffs_mount("/");

        auto file = yaffs_open("/file", O_CREAT | O_WRONLY, S_IRWXU);

        yaffs_write(file, buffer, sizeof(buffer));

        auto obj = yaffs_get_obj_from_fd(file);

        auto nand_chunk = yaffs_find_chunk_in_file(obj, 1, NULL);

        yaffs_close(file);

        yaffs_unmount("/");

        SwapBit(&driver.flash, nand_chunk * driver.geometry.chunkSize + 100, 0b11);

        yaffs_flush_whole_cache(&device, 1);

        yaffs_mount("/");

        file = yaffs_open("/file", O_RDONLY, S_IRWXU);

        memset(buffer, 0, sizeof(buffer));

        uint32_t unfixed = this->device.n_ecc_unfixed;

        yaffs_read(file, buffer, sizeof(buffer));

        ASSERT_THAT(this->device.n_ecc_unfixed - unfixed, Eq(1UL));

        yaffs_close(file);

        yaffs_unmount("/");
    }

    TEST_F(FileSystemTest, ShouldCreateDirectoryWithParents)
    {
        const char path[] = "/a/b/c/";

        yaffs_mount("/");

        api.MakeDirectory(path);

        ASSERT_THAT(api.Exists(path), Eq(true));

        yaffs_unmount("/");
    }

    TEST_F(FileSystemTest, ShouldCreateDirectoryAndParentExist)
    {
        const char path[] = "/a/b";

        yaffs_mount("/");

        yaffs_mkdir("/a", 0777);

        api.MakeDirectory(path);

        ASSERT_THAT(api.Exists(path), Eq(true));

        yaffs_unmount("/");
    }

    TEST_F(FileSystemTest, ShouldRemoveExistingFile)
    {
        const char path[] = "/file";

        yaffs_mount("/");

        auto file = yaffs_open(path, O_CREAT | O_WRONLY, S_IRWXU);

        yaffs_close(file);

        yaffs_unlink(path);

        ASSERT_THAT(api.Exists(path), Eq(false));

        yaffs_unmount("/");
    }

    TEST_F(FileSystemTest, ShouldRemoveNonExistingFile)
    {
        const char path[] = "/file";

        yaffs_mount("/");

        ASSERT_THAT(api.Unlink(path), Ne(OSResult::Success));

        ASSERT_THAT(api.Exists(path), Eq(false));

        yaffs_unmount("/");
    }

    TEST_F(FileSystemTest, ShouldMoveExistingFile)
    {
        const char* path = "/file";

        const char* target = "/target";

        yaffs_mount("/");

        auto file = yaffs_open(path, O_CREAT | O_WRONLY, S_IRWXU);

        yaffs_close(file);

        ASSERT_THAT(api.Move(path, target), Eq(OSResult::Success));

        ASSERT_THAT(api.Exists(path), Eq(false));
        ASSERT_THAT(api.Exists(target), Eq(true));

        yaffs_unmount("/");
    }

    TEST_F(FileSystemTest, MoveNonExistingFile)
    {
        const char* path = "/file";

        const char* target = "/target";

        yaffs_mount("/");

        ASSERT_THAT(api.Move(path, target), Ne(OSResult::Success));

        ASSERT_THAT(api.Exists(path), Eq(false));
        ASSERT_THAT(api.Exists(target), Eq(false));

        yaffs_unmount("/");
    }

    TEST_F(FileSystemTest, MoveToExistingFile)
    {
        const char* path = "/file";

        const char* target = "/target";

        yaffs_mount("/");

        yaffs_close(yaffs_open(path, O_CREAT | O_WRONLY, S_IRWXU));
        yaffs_close(yaffs_open(target, O_CREAT | O_WRONLY, S_IRWXU));

        ASSERT_THAT(api.Move(path, target), Eq(OSResult::Success));

        ASSERT_THAT(api.Exists(path), Eq(false));
        ASSERT_THAT(api.Exists(target), Eq(true));

        yaffs_unmount("/");
    }

    TEST_F(FileSystemTest, ShouldDetectDirectory)
    {
        const char path[] = "/a";

        yaffs_mount("/");

        yaffs_mkdir("/a", 0777);

        api.MakeDirectory(path);

        ASSERT_THAT(api.IsDirectory(path), Eq(true));

        yaffs_unmount("/");
    }

    TEST_F(FileSystemTest, ShouldNotDetectDirectoryOnFile)
    {
        const char path[] = "/file";

        yaffs_mount("/");

        yaffs_open(path, O_CREAT | O_WRONLY, S_IRWXU);

        ASSERT_THAT(api.IsDirectory(path), Eq(false));

        yaffs_unmount("/");
    }
}
