#ifndef LIBS_DRIVERS_N25Q_INCLUDE_N25Q_YAFFS_H_
#define LIBS_DRIVERS_N25Q_INCLUDE_N25Q_YAFFS_H_

#include "base/os.h"
#include "n25q.h"
#include "spi/spi.h"
#include "yaffs_guts.h"

namespace devices
{
    namespace n25q
    {
        enum class BlockMapping
        {
            SubSector,
            Sector
        };

        class N25QYaffsDeviceBase
        {
          public:
            N25QYaffsDeviceBase(const char* mountPoint,
                BlockMapping blockMapping,
                std::size_t chunkSize,
                std::size_t totalSize,
                drivers::spi::ISPIInterface& spi);

            N25QYaffsDeviceBase(const N25QYaffsDeviceBase&) = delete;

            OSResult Mount();
            OSResult EraseWholeChip();

            yaffs_dev* Device()
            {
                return &this->_device;
            }

          private:
            static int ReadChunk(struct yaffs_dev* dev, //
                int nand_chunk,
                u8* data,
                int data_len,
                u8* oob,
                int oob_len,
                enum yaffs_ecc_result* ecc_result);

            static int WriteChunk(struct yaffs_dev* dev, //
                int nand_chunk,
                const u8* data,
                int data_len,
                const u8* oob,
                int oob_len);

            static int EraseBlock(struct yaffs_dev* dev, int block_no);

            static int MarkBadBlock(struct yaffs_dev* dev, int block_no);

            static int CheckBadBlock(struct yaffs_dev* dev, int block_no);

            yaffs_dev _device;
            N25QDriver _driver;
            BlockMapping _blockMapping;
        };

        template <BlockMapping blockMapping, std::size_t ChunkSize, std::size_t TotalSize>
        class N25QYaffsDevice : public N25QYaffsDeviceBase
        {
          public:
            N25QYaffsDevice(const char* mountPoint, drivers::spi::ISPIInterface& spi)
                : N25QYaffsDeviceBase(mountPoint, blockMapping, ChunkSize, TotalSize, spi)
            {
            }
        };
    }
}

#endif /* LIBS_DRIVERS_N25Q_INCLUDE_N25Q_YAFFS_H_ */
