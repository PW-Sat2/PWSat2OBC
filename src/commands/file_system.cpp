#include <string.h>
#include <cstdint>
#include <gsl/span>
#include "base/os.h"
#include "obc.h"
#include "system.h"
#include "yaffs.hpp"

using std::uint16_t;
using std::int8_t;

void FSListFiles(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);

    const FSDirectoryOpenResult result = Main.fs.OpenDirectory(argv[0]);

    if (OS_RESULT_FAILED(result.Status))
    {
        Main.terminal.Puts("Error");
        Main.terminal.NewLine();
        return;
    }

    char* entry;
    FSDirectoryHandle dir = result.Handle;
    while ((entry = Main.fs.ReadDirectory(dir)) != NULL)
    {
        Main.terminal.Puts(entry);
        Main.terminal.NewLine();
    }

    Main.fs.CloseDirectory(dir);
}

void FSWriteFile(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);

    auto f = File::Open(Main.fs, argv[0], FSFileOpen::CreateAlways, FSFileAccess::WriteOnly);
    if (!f)
    {
        Main.terminal.Puts("Error");
        Main.terminal.NewLine();
        return;
    }

    f.Truncate(0);
    f.Write(gsl::make_span(reinterpret_cast<uint8_t*>(argv[1]), strlen(argv[1])));
}

void FSReadFile(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    auto f = File::Open(Main.fs, argv[0], FSFileOpen::Existing, FSFileAccess::ReadOnly);
    if (!f)
    {
        Main.terminal.Puts("Error");
        Main.terminal.NewLine();
        return;
    }

    std::array<uint8_t, 100> buffer;
    std::fill(buffer.begin(), buffer.end(), 0);

    f.Read(buffer);

    buffer[99] = 0;

    Main.terminal.Puts(reinterpret_cast<char*>(buffer.data()));
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
    UNUSED(argc, argv);

    auto r = Main.Storage.Erase();

    Main.terminal.Printf("Erase result: %d", num(r));
}

void SyncFS(uint16_t argc, char* argv[])
{
    UNUSED(argc, argv);

    Main.fs.Sync();
}
