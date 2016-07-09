#ifndef LIBS_STORAGE_NAND_H_
#define LIBS_STORAGE_NAND_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "storage.h"

/**
 * @defgroup nand-driver NAND Flash driver
 *
 * @brief Driver for NAND flash connected to EBI. Dedicated for flash on STK3700
 *
 * Driver assumes that:
 *  - EBI Bank0 is used
 *  - Single read/write is 32-bit
 *  - Offset to first page of first block is 0
 *  - Bad block is marked by value different that 0xFF in sixth byte of spare area in first page
 *
 * Also few functions related to flash geometry are provided to ease operating flash memory in bigger chunks
 *
 * @{
 */

/**
 * @brief Driver interface
 */
typedef struct _FlashNANDInterface
{
    /** @brief Context used by driver */
    void* context;
    /** @brief 8-bit data read/write address */
    uint8_t volatile* data8;
    /** @brief 16-bit data read/write address */
    uint16_t volatile* data16;
    /** @brief 32-bit data read/write address */
    uint32_t volatile* data32;
    /** @brief Address where offset within flash memory should be written */
    uint8_t volatile* addr;
    /** @brief Address where flash command should be written */
    uint8_t volatile* cmd;

    /**
     * @brief Pointer to procedure that initializes flash
     *
     * @param[out] flash Pointer to flash interface structure
     * @return Operation status
     */
    FlashStatus (*initialize)(struct _FlashNANDInterface* flash);

    /**
     * @brief Pointer to procedure that checks if flash is responding properly
     *
     * @param[in] flash Pointer to flash interface structure
     * @return Operation status
     */
    FlashStatus (*check)(struct _FlashNANDInterface* flash);

    /**
     * @brief Pointer to procedure that reads raw status
     *
     * @param[in] flash Pointer to flash interface structure
     * @return Raw status value
     */
    uint32_t (*status)(struct _FlashNANDInterface* flash);

    /**
     * Pointer to procedure that erases single block
     *
     * @param[in] flash Pointer to flash interface structure
     * @param[in] offset Offset to beginning of block that should be erased
     * @return Operations status
     */
    FlashStatus (*eraseBlock)(struct _FlashNANDInterface* flash, uint32_t offset);

    /**
     * @brief Pointer to procedure that reads single page
     *
     * @param[in] flash Pointer to flash interface structure
     * @param[in] offset Offset to beginning of page that should be read
     * @param[out] buffer Data buffer
     * @param[in] length Length of data to read. Must by multiply of 4
     * @return Operations status
     */
    FlashStatus (*readPage)(struct _FlashNANDInterface* flash, uint32_t offset, uint8_t* buffer, uint16_t length);

    /**
     * @brief Pointer to procedure that reads spare area from single page
     *
     * @param[in] flash Pointer to flash interface structure
     * @param[in] offset Offset to beginning of page which spare area should be read
     * @param[out] buffer Data buffer
     * @param[in] length Length of data to read. Must by multiply of 4
     * @return Operations status
     */
    FlashStatus (*readSpare)(struct _FlashNANDInterface* interface, uint32_t offset, uint8_t* buffer, uint16_t length);

    /**
     * @brief Pointer to procedure that writes single page
     *
     * @param[in] flash Pointer to flash interface structure
     * @param[in] offset Offset to beginning of page that should be written
     * @param[in] buffer Data buffer
     * @param[in] length Length of data to written. Must by multiply of 4
     * @return Operations status
     */
    FlashStatus (*writePage)(
        struct _FlashNANDInterface* interface, uint32_t offset, const uint8_t* buffer, uint32_t length);

    /**
    * @brief Pointer to procedure that writes spare area of single page
    *
    * @param[in] flash Pointer to flash interface structure
    * @param[in] offset Offset to beginning of page which spare area should be written
    * @param[in] buffer Data buffer
    * @param[in] length Length of data to written. Must by multiply of 4
    * @return Operations status
    */
    FlashStatus (*writeSpare)(
        struct _FlashNANDInterface* interface, uint32_t offset, const uint8_t* buffer, uint16_t length);

    /**
     * @brief Pointer to procedure that checks if block is marked as bad
     *
     * @param[in] flash Pointer to flash interface structure
     * @param[in] offset Offset to beginning of block that should be checked
     * @return true if block is marked as bad
     */
    bool (*isBadBlock)(struct _FlashNANDInterface* flash, uint32_t offset);

    /**
     * @brief Pointer to procedure that marks block as bad
     *
     * @param[in] flash Pointer to flash interface structure
     * @param[in] offset Offset to beginning of block that should be marked
     * @return Operation status
     */
    FlashStatus (*markBadBlock)(struct _FlashNANDInterface* flash, uint32_t offset);
} FlashNANDInterface;

/**
 * @brief Initializes flash interface structure
 * @param[out] flash Pointer to structure
 */
void BuildNANDInterface(FlashNANDInterface* flash);

/**
 * @brief Describes geometry of flash memory
 */
typedef struct
{
    /** @brief Base offset */
    uint32_t baseOffset;
    /** @brief Size of single page */
    uint16_t pageSize;
    /** @brief Count of pages in block */
    uint8_t pagesPerBlock;
    /** @brief Size of spare area of single page */
    uint8_t spareAreaPerPage;
    /** @brief Coung of pages in single chunk */
    uint8_t pagesPerChunk;
    /** @brief Calculated. Size of chunk */
    uint32_t chunkSize;
    /** @brief Calculated. Size of block */
    uint32_t blockSize;
    /** @brief Calculated. Count of chunks in block */
    uint32_t chunksPerBlock;
} NANDGeometry;

/**
 * @brief Structure describing single operation (read or write) on flash memory
 */
typedef struct
{
    /** @brief Offset to point where operation should start */
    uint32_t offset;
    /** @brief Size of operation in data-area of pages */
    uint32_t dataSize;
    /** @brief Data buffer */
    uint8_t* dataBuffer;
    /** @brief Size of operation in spare-area of pages */
    uint32_t spareSize;
    /** @brief Spare buffer */
    uint8_t* spareBuffer;
} NANDOperation;

/**
 * @brief Structure describing single slice (part) of operation. Each slice affected single page
 */
typedef struct
{
    /** @brief Offset ot poing where slice should start */
    uint32_t offset;
    /** @brief Size of slice in data-area of pages */
    uint32_t dataSize;
    /** @brief Data buffer */
    uint8_t* dataBuffer;
    /** @brief Size of slice in spare-area of pages */
    uint32_t spareSize;
    /** @brief Spare buffer */
    uint8_t* spareBuffer;
} NANDOperationSlice;

/**
 * @brief Calculates remaining geometry values
 * @param[inout] geometry Flash geometry
 */
void NANDCalculateGeometry(NANDGeometry* geometry);

/**
 * @brief Calculates offset to start of first page in chunk
 * @param[in] geometry Flash geometry
 * @param[in] chunkNo Chunk number (indexed from 0)
 * @return Offset to first page in chunk
 */
uint32_t NANDPageOffsetFromChunk(NANDGeometry* geometry, uint16_t chunkNo);

/**
 * @brief Calculates offset to start of first page in block
 * @param[in] geometry Flash geometry
 * @param[in] blockNo Block number (indexed from 0)
 * @return Offset to first page in block
 */
uint32_t NANDBlockOffset(NANDGeometry* geometry, uint16_t blockNo);

/**
 * @brief Calculates number of pages affected by single operation
 * @param[in] geometry Flash geometry
 * @param[in] operation Operation definition
 * @return Number of pages affected by operation
 */
uint16_t NANDAffectedPagesCount(NANDGeometry* geometry, NANDOperation* operation);

/**
 * @brief Calculates slice of operation for given page
 * @param[in] geometry Flash geometry
 * @param[in] operation Operation definition
 * @param[in] pageNo Page number
 * @return Operation slice affecting given page
 */
NANDOperationSlice NANDGetOperationSlice(NANDGeometry* geometry, NANDOperation* operation, uint16_t pageNo);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* LIBS_STORAGE_NAND_H_ */
