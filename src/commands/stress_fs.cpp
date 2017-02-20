#include <array>
#include <cstdint>
#include "fs/fs.h"
#include "gsl/span"
#include "obc.h"
#include "yaffs.hpp"

static bool IsDiff(gsl::span<int> prev, gsl::span<int> next)
{
    for (auto i = 0; i < YAFFS_NUMBER_OF_BLOCK_STATES; i++)
    {
        if (prev[i] != next[i])
        {
            return true;
        }
    }

    return false;
}

extern "C" {
struct yaffs_tnode_list
{
    struct yaffs_tnode_list* next;
    struct yaffs_tnode* tnodes;
};

struct yaffs_obj_list
{
    struct yaffs_obj_list* next;
    struct yaffs_obj* objects;
};
}

struct yaffs_allocator
{
    int n_tnodes_created;
    struct yaffs_tnode* free_tnodes;
    int n_free_tnodes;
    struct yaffs_tnode_list* alloc_tnode_list;

    int n_obj_created;
    struct list_head free_objs;
    int n_free_objects;

    struct yaffs_obj_list* allocated_obj_list;
};

static std::array<std::uint8_t, 1024> buffer;

void StressFS(std::uint16_t /*argc*/, char* /*argv*/ [])
{
    for (decltype(buffer.size()) i = 0; i < buffer.size(); i++)
    {
        buffer[i] = i % 256;
    }

    auto& flash = Main.Storage._storage._flashes[0];

    auto dev = flash.Device.Device();
    auto alloc = reinterpret_cast<yaffs_allocator*>(dev->allocator);

    int prev[YAFFS_NUMBER_OF_BLOCK_STATES] = {0};
    int next[YAFFS_NUMBER_OF_BLOCK_STATES] = {0};

    Main.terminal.Puts("Emp\tAlloc\tFull\tDirty\t   FT\t   CT\t   FO\t   CO\n");

    int file_no = 0;

    while (true)
    {
        char buf[20];

        sprintf(buf, "/a/big%d.dat", file_no);
        services::fs::File f(Main.fs, buf, services::fs::FileOpen::CreateAlways, services::fs::FileAccess::ReadWrite);

        if (!f)
        {
            Main.terminal.Puts("error");
            return;
        }

        for (auto i = 0; i < 1000; i++)
        {
            auto r = f.Write(buffer);

            if (!r)
            {
                Main.terminal.Printf("Write error = %d", num(r.Status));
                return;
            }

            yaffs_count_blocks_by_state(dev, next);
            auto empty = next[YAFFS_BLOCK_STATE_EMPTY];
            auto allocating = next[YAFFS_BLOCK_STATE_ALLOCATING];
            auto full = next[YAFFS_BLOCK_STATE_FULL];
            auto dirty = next[YAFFS_BLOCK_STATE_DIRTY];

            if (IsDiff(prev, next))
            {
                std::copy(std::begin(next), std::end(next), std::begin(prev));
                Main.terminal.Printf( //
                    "%3d\t%5d\t%4d\t%5d\t%5d\t%5d\t%5d\t%5d\n",
                    empty,
                    allocating,
                    full,
                    dirty,
                    alloc->n_free_tnodes,
                    alloc->n_tnodes_created,
                    alloc->n_free_objects,
                    alloc->n_tnodes_created);
            }
        }

        file_no++;
    }
}
