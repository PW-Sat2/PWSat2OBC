#include <string.h>
#include <cstdint>
#include "base/os.h"
#include "obc.h"
#include "system.h"

extern "C" {
#include "yaffs_guts.h"
#include "yaffsfs.h"
}

using std::uint16_t;
using std::int8_t;

void FSListFiles(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);

    const FSDirectoryOpenResult result = Main.fs.openDirectory(&Main.fs, argv[0]);

    if (OS_RESULT_FAILED(result.Status))
    {
        Main.terminal.Puts("Error");
        Main.terminal.NewLine();
        return;
    }

    char* entry;
    FSDirectoryHandle dir = result.Handle;
    while ((entry = Main.fs.readDirectory(&Main.fs, dir)) != NULL)
    {
        Main.terminal.Puts(entry);
        Main.terminal.NewLine();
    }

    Main.fs.closeDirectory(&Main.fs, dir);
}

void FSWriteFile(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);

    const FSFileOpenResult result = Main.fs.open(&Main.fs, argv[0], FsOpenCreateAlways, FsWriteOnly);
    if (OS_RESULT_FAILED(result.Status))
    {
        Main.terminal.Puts("Error");
        Main.terminal.NewLine();
        return;
    }

    const FSFileHandle file = result.Handle;
    Main.fs.ftruncate(&Main.fs, file, 0);
    Main.fs.write(&Main.fs, file, argv[1], strlen(argv[1]));
    Main.fs.close(&Main.fs, file);
}

void FSReadFile(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    const FSFileOpenResult result = Main.fs.open(&Main.fs, argv[0], FsOpenExisting, FsReadOnly);
    if (OS_RESULT_FAILED(result.Status))
    {
        Main.terminal.Puts("Error");
        Main.terminal.NewLine();
        return;
    }

    char buffer[100];
    memset(buffer, 0, sizeof(buffer));
    const FSFileHandle file = result.Handle;
    Main.fs.read(&Main.fs, file, buffer, sizeof(buffer));
    Main.fs.close(&Main.fs, file);

    buffer[99] = 0;

    Main.terminal.Puts(buffer);
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

    Main.fs.makeDirectory(&Main.fs, argv[0]);
}

void EraseFlash(uint16_t argc, char* argv[])
{
    UNUSED(argc, argv);

#ifdef USE_EXTERNAL_FLASH
    auto result = Main.Storage.ExternalFlashDriver.EraseChip();

    Main.terminal.Printf("Erase result: %d", num(result));
#else
    Main.terminal.Printf("No erase for internal flash");
#endif
}

void SyncFS(uint16_t argc, char* argv[])
{
    UNUSED(argc, argv);

    Main.fs.Sync(&Main.fs);
}
