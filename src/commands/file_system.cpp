#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <gsl/span>
#include "base/os.h"
#include "base/reader.h"
#include "base/writer.h"
#include "fs/yaffs.h"
#include "obc.h"
#include "obc_access.hpp"
#include "system.h"
#include "terminal/terminal.h"
#include "yaffs.hpp"

using std::uint16_t;
using std::int8_t;
using services::fs::File;
using services::fs::DirectoryOpenResult;
using services::fs::FileOpen;
using services::fs::FileAccess;
using services::fs::DirectoryHandle;

void FSListFiles(uint16_t argc, char* argv[])
{
    if (argc != 1)
    {
        GetTerminal().Puts("listFiles <path>\n");
        return;
    }

    const DirectoryOpenResult result = GetFileSystem().OpenDirectory(argv[0]);

    if (OS_RESULT_FAILED(result.Status))
    {
        GetTerminal().Puts("Error");
        GetTerminal().NewLine();
        return;
    }

    char* entry;
    DirectoryHandle dir = result.Result;
    while ((entry = GetFileSystem().ReadDirectory(dir)) != NULL)
    {
        auto l = strnlen(entry, 80);

        if (entry[l + 1] != '\0')
        {
            GetTerminal().Puts("[lost file]");
        }
        else
        {
            GetTerminal().Puts(entry);
        }
        GetTerminal().NewLine();
    }

    GetFileSystem().CloseDirectory(dir);
}

void FSWriteFile(uint16_t argc, char* argv[])
{
    if (argc != 1)
    {
        GetTerminal().Puts("writeFile <path>\n");
        return;
    }

    std::array<uint8_t, 256> partBuffer;

    TerminalPartialRetrival retr(GetTerminal(), partBuffer);

    retr.Start();

    File f(GetFileSystem(), argv[0], FileOpen::CreateAlways, FileAccess::WriteOnly);
    if (!f)
    {
        GetTerminal().Puts("Error");
        GetTerminal().NewLine();
        return;
    }

    f.Truncate(0);

    std::size_t written = 0;

    while (true)
    {
        auto part = retr.ReadPart();

        if (!part.HasValue)
            break;

        auto ioResult = f.Write(part.Value);

        written += ioResult.Result.size();
    }

    GetTerminal().Printf("%d\n", written);
}

void FSReadFile(uint16_t argc, char* argv[])
{
    if (argc != 1)
    {
        GetTerminal().Puts("readFile <path>\n");
        return;
    }

    File f(GetFileSystem(), argv[0], FileOpen::Existing, FileAccess::ReadOnly);
    if (!f)
    {
        GetTerminal().Puts("Error");
        GetTerminal().NewLine();
        return;
    }

    char buf[10] = {0};
    itoa(f.Size(), buf, 10);

    GetTerminal().Puts(buf);
    GetTerminal().NewLine();

    std::array<uint8_t, 1000> buffer;

    while (true)
    {
        auto r = f.Read(buffer);
        if (!r || r.Result.size() == 0)
        {
            break;
        }

        GetTerminal().PrintBuffer(r.Result);
    }

    GetTerminal().NewLine();
}

void MountList(uint16_t argc, char* argv[])
{
    UNUSED(argc, argv);

    yaffs_dev_rewind();

    yaffs_dev* dev = nullptr;

    GetTerminal().Puts("Mountpoint     \t    Free\t   Total\tEmp\tAlloc\tFull\tDirty\n");

    while ((dev = yaffs_next_dev()) != nullptr)
    {
        auto totalSize = static_cast<uint32_t>(yaffs_totalspace(dev->param.name));
        auto freeSize = static_cast<uint32_t>(yaffs_freespace(dev->param.name));

        int states[YAFFS_NUMBER_OF_BLOCK_STATES] = {0};

        yaffs_count_blocks_by_state(dev, states);

        auto empty = states[YAFFS_BLOCK_STATE_EMPTY];
        auto allocating = states[YAFFS_BLOCK_STATE_ALLOCATING];
        auto full = states[YAFFS_BLOCK_STATE_FULL];
        auto dirty = states[YAFFS_BLOCK_STATE_DIRTY];

        GetTerminal().Printf( //
            "%-15s\t%8ld\t%8ld\t%3d\t%5d\t%4d\t%5d\n",
            dev->param.name,
            freeSize,
            totalSize,
            empty,
            allocating,
            full,
            dirty);
    }
}

void MakeDirectory(uint16_t argc, char* argv[])
{
    if (argc != 1)
    {
        GetTerminal().Puts("mkdir <path>\n");

        return;
    }

    GetFileSystem().MakeDirectory(argv[0]);
}

void EraseFlash(uint16_t argc, char* argv[])
{
    if (argc == 0)
    {
        GetTerminal().Puts("erase <all|[0|1|2]>");
        return;
    }

    if (strcmp(argv[0], "all") == 0)
    {
        GetTerminal().Puts("Erasing all flashes ...");
        GetTerminal().NewLine();

        yaffsfs_Lock();

        auto r = Main.Storage.Erase();

        yaffsfs_Unlock();
        GetTerminal().Printf("Erase result: %d", num(r));
    }
    else
    {
#ifdef USE_EXTERNAL_FLASH
        auto flashIndex = strtol(argv[0], nullptr, 10);
        if (flashIndex < 0 || flashIndex >= 3)
        {
            GetTerminal().Puts("Flash index must in range <0;2>");
            return;
        }

        GetTerminal().Printf("Erasing single flash: %ld ...", flashIndex);
        GetTerminal().NewLine();

        auto flash = Main.Storage.GetInternalStorage().GetDriver(flashIndex);
        auto r = flash.BeginEraseChip().Wait();

        GetTerminal().Printf("Erase result: %d ...", num(r));
#else
        GetTerminal().Puts("Erasing single chip is not supported on STK Storage");
        GetTerminal().NewLine();
#endif
    }
}

void SyncFS(uint16_t argc, char* argv[])
{
    UNUSED(argc, argv);

    GetFileSystem().Sync();
}

void RemoveFile(uint16_t /*argc*/, char* argv[])
{
    const char* path = argv[0];

    auto r = yaffs_unlink(path);

    LOGF(LOG_LEVEL_INFO, "Ret=%d", r);
}
