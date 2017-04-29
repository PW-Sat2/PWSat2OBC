#ifndef LIBS_DRIVERS_N25Q_INCLUDE_N25Q_YAFFS_H_
#define LIBS_DRIVERS_N25Q_INCLUDE_N25Q_YAFFS_H_

#include "base/os.h"
#include "fs/yaffs.h"
#include "logger/logger.h"
#include "n25q.h"
#include "spi/spi.h"
#include "yaffs.hpp"

namespace devices
{
    namespace n25q
    {
        /**
         * @defgroup n25q_yaffs Yaffs driver for N25Q flash memory
         * @ingroup n25q
         *
         * This module contains Yaffs driver for N25Q flash memory
         *
         * @{
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
         * @tparam blockMapping Block mapping
         * @tparam ChunkSize Single chunk size
         * @tparam TotalSize Total memory size
         */
        template <BlockMapping blockMapping, std::size_t ChunkSize, std::size_t TotalSize> class N25QYaffsDevice
        {
          public:
            /**
             * @brief Constructs @ref N25QYaffsDevice instance
             * @param[in] mountPoint Mount point (absolute path)
             * @param[in] driver N25Q driver to use
             */
            N25QYaffsDevice(const char* mountPoint, RedundantN25QDriver& driver);

            /**
             * @brief Mounts device
             * @param deviceOperations YAFFS device operations
             * @return Operation result
             */
            OSResult Mount(services::fs::IYaffsDeviceOperations& deviceOperations);

            /** @brief Return raw yaffs device */
            inline yaffs_dev* Device();

          private:
            N25QYaffsDevice(const N25QYaffsDevice&) = delete;
            N25QYaffsDevice& operator=(const N25QYaffsDevice&) = delete;
            N25QYaffsDevice(N25QYaffsDevice&&) = delete;
            N25QYaffsDevice& operator=(N25QYaffsDevice&&) = delete;

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
            RedundantN25QDriver& _driver;
            /** @brief Block mapping */
            const BlockMapping _blockMapping;
            /** @brief First buffer for redundant reads */
            std::array<std::uint8_t, ChunkSize> _redundantReadBuffer1;
            /** @brief Second buffer for redundant reads */
            std::array<std::uint8_t, ChunkSize> _redundantReadBuffer2;
        };

        template <BlockMapping blockMapping> struct BlockSize;
        template <> struct BlockSize<BlockMapping::Sector>
        {
            static constexpr size_t value = 64_KB;
        };
        template <> struct BlockSize<BlockMapping::SubSector>
        {
            static constexpr size_t value = 4_KB;
        };

        template <BlockMapping blockMapping, std::size_t ChunkSize, std::size_t TotalSize>
        N25QYaffsDevice<blockMapping, ChunkSize, TotalSize>::N25QYaffsDevice(const char* mountPoint, RedundantN25QDriver& driver)
            : _driver(driver), //
              _blockMapping(blockMapping)
        {
            memset(&this->_device, 0, sizeof(this->_device));

            this->_device.param.name = mountPoint;

            this->_device.param.inband_tags = true;
            this->_device.param.is_yaffs2 = true;
            this->_device.param.total_bytes_per_chunk = ChunkSize;
            this->_device.param.chunks_per_block = BlockSize<blockMapping>::value / this->_device.param.total_bytes_per_chunk;
            this->_device.param.spare_bytes_per_chunk = 0;
            this->_device.param.start_block = 1;
            this->_device.param.n_reserved_blocks = 3;
            this->_device.param.no_tags_ecc = true;
            this->_device.param.always_check_erased = true;
            this->_device.param.disable_bad_block_marking = true;

            this->_device.driver_context = this;
            this->_device.drv.drv_read_chunk_fn = N25QYaffsDevice<blockMapping, ChunkSize, TotalSize>::ReadChunk;
            this->_device.drv.drv_write_chunk_fn = N25QYaffsDevice<blockMapping, ChunkSize, TotalSize>::WriteChunk;
            this->_device.drv.drv_erase_fn = N25QYaffsDevice<blockMapping, ChunkSize, TotalSize>::EraseBlock;
            this->_device.drv.drv_mark_bad_fn = N25QYaffsDevice<blockMapping, ChunkSize, TotalSize>::MarkBadBlock;
            this->_device.drv.drv_check_bad_fn = N25QYaffsDevice<blockMapping, ChunkSize, TotalSize>::CheckBadBlock;

            auto blockSize = this->_device.param.chunks_per_block * this->_device.param.total_bytes_per_chunk;

            this->_device.param.end_block = TotalSize / blockSize //
                - this->_device.param.start_block                 //
                - this->_device.param.n_reserved_blocks;
        }

        template <BlockMapping blockMapping, std::size_t ChunkSize, std::size_t TotalSize>
        OSResult N25QYaffsDevice<blockMapping, ChunkSize, TotalSize>::Mount(services::fs::IYaffsDeviceOperations& deviceOperations)
        {
            auto result = deviceOperations.AddDeviceAndMount(&this->_device);
            if (OS_RESULT_SUCCEEDED(result))
            {
                LOGF(LOG_LEVEL_INFO, "[Device %s] Mounted successfully", this->_device.param.name);
                return OSResult::Success;
            }
            else
            {
                LOGF(LOG_LEVEL_ERROR, "[Device %s] Mount failed: %d", this->_device.param.name, num(result));
                return result;
            }
        }

        template <BlockMapping blockMapping, std::size_t ChunkSize, std::size_t TotalSize>
        yaffs_dev* N25QYaffsDevice<blockMapping, ChunkSize, TotalSize>::Device()
        {
            return &this->_device;
        }

        template <BlockMapping blockMapping, std::size_t ChunkSize, std::size_t TotalSize>
        int N25QYaffsDevice<blockMapping, ChunkSize, TotalSize>::ReadChunk(struct yaffs_dev* dev, //
            int nand_chunk,
            u8* data,
            int data_len,
            u8* oob,
            int oob_len,
            enum yaffs_ecc_result* ecc_result)
        {
            UNREFERENCED_PARAMETER(oob);

            if (oob_len > 0)
            {
                LOGF(LOG_LEVEL_ERROR, "Trying to read OOB %d bytes", oob_len);
                return YAFFS_FAIL;
            }

            if (data_len > static_cast<int>(ChunkSize))
            {
                LOGF(LOG_LEVEL_ERROR, "Trying to read to large page: %d bytes", data_len);
                return YAFFS_FAIL;
            }

            auto This = reinterpret_cast<N25QYaffsDevice<blockMapping, ChunkSize, TotalSize>*>(dev->driver_context);

            *ecc_result = yaffs_ecc_result::YAFFS_ECC_RESULT_NO_ERROR;

            auto baseAddress = nand_chunk * dev->param.total_bytes_per_chunk;

            gsl::span<uint8_t> outputBuffer(data, data_len);
            gsl::span<uint8_t> redundantBuffer1(This->_redundantReadBuffer1.data(), data_len);
            gsl::span<uint8_t> redundantBuffer2(This->_redundantReadBuffer2.data(), data_len);

            This->_driver.ReadMemory(baseAddress, outputBuffer, redundantBuffer1, redundantBuffer2);

            return YAFFS_OK;
        }

        template <BlockMapping blockMapping, std::size_t ChunkSize, std::size_t TotalSize>
        int N25QYaffsDevice<blockMapping, ChunkSize, TotalSize>::WriteChunk(struct yaffs_dev* dev, //
            int nand_chunk,
            const u8* data,
            int data_len,
            const u8* oob,
            int oob_len)
        {
            UNREFERENCED_PARAMETER(oob);

            if (oob_len > 0)
            {
                LOGF(LOG_LEVEL_ERROR, "Trying to write OOB %d bytes", oob_len);
                return YAFFS_FAIL;
            }

            auto This = reinterpret_cast<N25QYaffsDevice*>(dev->driver_context);

            auto baseAddress = nand_chunk * dev->param.total_bytes_per_chunk;

            gsl::span<const uint8_t> buffer(data, data_len);

            auto result = This->_driver.WriteMemory(baseAddress, buffer);

            if (result != OperationResult::Success)
            {
                LOGF(LOG_LEVEL_ERROR, "[Device %s] Write to chunk %d failed Error %d", dev->param.name, nand_chunk, num(result));
                return YAFFS_FAIL;
            }

            return YAFFS_OK;
        }

        template <BlockMapping blockMapping, std::size_t ChunkSize, std::size_t TotalSize>
        int N25QYaffsDevice<blockMapping, ChunkSize, TotalSize>::EraseBlock(struct yaffs_dev* dev, int block_no)
        {
            auto This = reinterpret_cast<N25QYaffsDevice*>(dev->driver_context);

            LOGF(LOG_LEVEL_INFO, "[Device %s] Erasing block %d", dev->param.name, block_no);

            auto baseAddress = block_no * dev->param.chunks_per_block * dev->param.total_bytes_per_chunk;

            auto result = OperationResult::Failure;

            switch (This->_blockMapping)
            {
                case devices::n25q::BlockMapping::Sector:
                    result = This->_driver.EraseSector(baseAddress);
                    break;

                case devices::n25q::BlockMapping::SubSector:
                    result = This->_driver.EraseSubSector(baseAddress);
                    break;
            }

            if (result != OperationResult::Success)
            {
                LOGF(LOG_LEVEL_ERROR, "[Device %s] Erase block failed: %d Error %d", dev->param.name, block_no, num(result));
                return YAFFS_FAIL;
            }

            return YAFFS_OK;
        }

        template <BlockMapping blockMapping, std::size_t ChunkSize, std::size_t TotalSize>
        int N25QYaffsDevice<blockMapping, ChunkSize, TotalSize>::MarkBadBlock(struct yaffs_dev* dev, int block_no)
        {
            UNREFERENCED_PARAMETER(dev);
            LOGF(LOG_LEVEL_WARNING, "[Device %s] Marking bad block %d", dev->param.name, block_no);

            return YAFFS_OK;
        }

        template <BlockMapping blockMapping, std::size_t ChunkSize, std::size_t TotalSize>
        int N25QYaffsDevice<blockMapping, ChunkSize, TotalSize>::CheckBadBlock(struct yaffs_dev* dev, int block_no)
        {
            UNUSED(dev, block_no);

            return YAFFS_OK;
        }

        /** @} */
    }
}

#endif /* LIBS_DRIVERS_N25Q_INCLUDE_N25Q_YAFFS_H_ */
