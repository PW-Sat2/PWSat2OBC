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
#include "obc.h"
#include "system.h"
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
        Main.terminal.Puts("listFiles <path>\n");
        return;
    }

    const DirectoryOpenResult result = Main.fs.OpenDirectory(argv[0]);

    if (OS_RESULT_FAILED(result.Status))
    {
        Main.terminal.Puts("Error");
        Main.terminal.NewLine();
        return;
    }

    char* entry;
    DirectoryHandle dir = result.Result;
    while ((entry = Main.fs.ReadDirectory(dir)) != NULL)
    {
        Main.terminal.Puts(entry);
        Main.terminal.NewLine();
    }

    Main.fs.CloseDirectory(dir);
}

void FSWriteFile(uint16_t argc, char* argv[])
{
    if (argc != 1)
    {
        Main.terminal.Puts("writeFile <path>\n");
        return;
    }

    std::array<uint8_t, 256> partBuffer;

    TerminalPartialRetrival retr(Main.terminal, partBuffer);

    retr.Start();

    File f(Main.fs, argv[0], FileOpen::CreateAlways, FileAccess::WriteOnly);
    if (!f)
    {
        Main.terminal.Puts("Error");
        Main.terminal.NewLine();
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

    Main.terminal.Printf("%d\n", written);
}

void FSReadFile(uint16_t argc, char* argv[])
{
    if (argc != 1)
    {
        Main.terminal.Puts("readFile <path>\n");
        return;
    }

    File f(Main.fs, argv[0], FileOpen::Existing, FileAccess::ReadOnly);
    if (!f)
    {
        Main.terminal.Puts("Error");
        Main.terminal.NewLine();
        return;
    }

    char buf[10] = {0};
    itoa(f.Size(), buf, 10);

    Main.terminal.Puts(buf);
    Main.terminal.NewLine();

    std::array<uint8_t, 100> buffer;

    while (true)
    {
        auto r = f.Read(buffer);
        if (!r || r.Result.size() == 0)
        {
            break;
        }

        Main.terminal.PrintBuffer(r.Result);
    }

    Main.terminal.NewLine();
}

void MountList(uint16_t argc, char* argv[])
{
    UNUSED(argc, argv);

    yaffs_dev_rewind();

    yaffs_dev* dev = nullptr;

    Main.terminal.Puts("Mountpoint     \t    Free\t   Total\tEmp\tAlloc\tFull\tDirty\n");

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

        Main.terminal.Printf( //
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
        Main.terminal.Puts("mkdir <path>\n");

        return;
    }

    Main.fs.MakeDirectory(argv[0]);
}

void EraseFlash(uint16_t argc, char* argv[])
{
    if (argc == 0)
    {
        Main.terminal.Puts("erase <all|[0|1|2]>");
        return;
    }

    if (strcmp(argv[0], "all") == 0)
    {
        Main.terminal.Puts("Erasing all flashes ...");
        Main.terminal.NewLine();

        auto r = Main.Storage.Erase();
        Main.terminal.Printf("Erase result: %d", num(r));
    }
    else
    {
#ifdef USE_EXTERNAL_FLASH
        auto flashIndex = strtol(argv[0], nullptr, 10);
        if (flashIndex < 0 || flashIndex >= 3)
        {
            Main.terminal.Puts("Flash index must in range <0;2>");
            return;
        }

        Main.terminal.Printf("Erasing single flash: %ld ...", flashIndex);
        Main.terminal.NewLine();

        auto flash = Main.Storage.GetInternalStorage().GetDriver(flashIndex);
        auto r = flash.BeginEraseChip().Wait();

        Main.terminal.Printf("Erase result: %d ...", num(r));
#else
        Main.terminal.Puts("Erasing single chip is not supported on STK Storage");
        Main.terminal.NewLine();
#endif
    }
}

void SyncFS(uint16_t argc, char* argv[])
{
    UNUSED(argc, argv);

    Main.fs.Sync();
}

void RemoveFile(uint16_t /*argc*/, char* argv[])
{
    const char* path = argv[0];

    auto r = yaffs_unlink(path);

    LOGF(LOG_LEVEL_INFO, "Ret=%d", r);
}
