#ifndef LIBS_DRIVERS_N25Q_INCLUDE_N25Q_YAFFS_H_
#define LIBS_DRIVERS_N25Q_INCLUDE_N25Q_YAFFS_H_

#include "base/os.h"
#include "n25q.h"
#include "spi/spi.h"
extern "C" {
#include "yaffs_guts.h"
}

namespace devices
{
    namespace n25q
    {
        /**
         * @defgroup n25q_yaffs Yaffs driver for N25Q flash memory
         * @ingroup n25q
         *
         * This module contains Yaffs driver for N25Q flash memory
         */

        /**
         * @brief Possible mappings of Yaffs block to memory structures
         */
        enum class BlockMapping
        {
            SubSector, //!< SubSector
            Sector     //!< Sector
        };

        /**
         * @brief Yaffs driver for N25Q flash memory
         */
        class N25QYaffsDeviceBase
        {
          public:
            /**
             * @brief Constructs @ref N25QYaffsDeviceBase instance
             * @param[in] mountPoint Mount point (absolute path)
             * @param[in] blockMapping Block mapping
             * @param[in] chunkSize Single chunk size
             * @param[in] totalSize Total memory size
             * @param[in] driver N25Q driver to use
             */
            N25QYaffsDeviceBase(const char* mountPoint,
                const BlockMapping blockMapping,
                const std::size_t chunkSize,
                const std::size_t totalSize,
                N25QDriver& driver);

            /**
             * @brief Mounts device
             * @return Operation result
             */
            OSResult Mount();

            /** @brief Return raw yaffs device */
            inline yaffs_dev* Device();

          private:
            N25QYaffsDeviceBase(const N25QYaffsDeviceBase&) = delete;
            N25QYaffsDeviceBase& operator=(const N25QYaffsDeviceBase&) = delete;
            N25QYaffsDeviceBase(N25QYaffsDeviceBase&&) = delete;
            N25QYaffsDeviceBase& operator=(N25QYaffsDeviceBase&&) = delete;

            /**
             * @brief (Yaffs callback) Reads chunk from memory
             * @param[in] dev Yaffs device
             * @param[in] nand_chunk Chunk number
             * @param[out] data Data buffer
             * @param[in] data_len Data length
             * @param[out] oob OOB buffer (not supported)
             * @param[in] oob_len OOB buffer length
             * @param[out] ecc_result ECC result
             * @return Operation result
             */
            static int ReadChunk(struct yaffs_dev* dev, //
                int nand_chunk,
                u8* data,
                int data_len,
                u8* oob,
                int oob_len,
                enum yaffs_ecc_result* ecc_result);

            /**
             * @brief (Yaffs callback) Writes chunk to memory
             * @param[in] dev Yaffs device
             * @param[in] nand_chunk Chunk number
             * @param[in] data Data buffer
             * @param[in] data_len Data length
             * @param[in] oob OOB buffer (not supported)
             * @param[in] oob_len OOB buffer length
             * @return Operation result
             */
            static int WriteChunk(struct yaffs_dev* dev, //
                int nand_chunk,
                const u8* data,
                int data_len,
                const u8* oob,
                int oob_len);

            /**
             * @brief (Yaffs callback) Erases block
             * @param[in] dev Yaffs device
             * @param[in] block_no Block number
             * @return Operation result
             */
            static int EraseBlock(struct yaffs_dev* dev, int block_no);

            /**
             * @brief (Yaffs callback) Marks block as bad
             * @param[in] dev Yaffs device
             * @param[in] block_no Block number
             * @return Operation result
             */
            static int MarkBadBlock(struct yaffs_dev* dev, int block_no);

            /**
            * @brief (Yaffs callback) Checks if block is marked as bad
            * @param[in] dev Yaffs device
            * @param[in] block_no Block number
            * @return true of block is marked as bad
            */
            static int CheckBadBlock(struct yaffs_dev* dev, int block_no);

            /** @brief Yaffs device */
            yaffs_dev _device;
            /** @brief Low-level N25Q driver */
            N25QDriver& _driver;
            /** @brief Block mapping */
            const BlockMapping _blockMapping;
        };

        inline yaffs_dev* N25QYaffsDeviceBase::Device()
        {
            return &this->_device;
        }

        /**
         * @brief Helper class allowing to specify @ref N25QYaffsDeviceBase parameters as template arguments
         */
        template <BlockMapping blockMapping, std::size_t ChunkSize, std::size_t TotalSize>
        class N25QYaffsDevice final : public N25QYaffsDeviceBase
        {
          public:
            /**
             * @brief Constructs @ref N25QYaffsDevice
             * @param[in] mountPoint Mount point
             * @param[in] driver N25Q driver to use
             */
            N25QYaffsDevice(const char* mountPoint, N25QDriver& driver)
                : N25QYaffsDeviceBase(mountPoint, blockMapping, ChunkSize, TotalSize, driver)
            {
            }
        };
    }
}

#endif /* LIBS_DRIVERS_N25Q_INCLUDE_N25Q_YAFFS_H_ */
