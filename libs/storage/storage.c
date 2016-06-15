#include <string.h>
#include <em_cmu.h>
#include <em_ebi.h>
#include <em_gpio.h>

#include "Logger/logger.h"

#include "storage.h"

#define NAND_POWER_PORT gpioPortB
#define NAND_POWER_PIN 15
#define NAND_READY_PORT gpioPortD
#define NAND_READY_PIN 15
#define NAND_CE_PORT gpioPortD
#define NAND_CE_PIN 14
#define NAND_WP_PORT gpioPortD
#define NAND_WP_PIN 13
#define NAND_ALE_BIT 24
#define NAND_CLE_BIT 25

#define NAND256W3A_SIGNATURE 0x7520
#define NAND256W3A_SIZE (32 * 1024 * 1024)
#define NAND256W3A_PAGESIZE 512
#define NAND256W3A_BLOCKSIZE (16 * 1024)

// generic for NAND
#define NAND_PAGEADDR_MASK (NAND256W3A_PAGESIZE - 1)
#define NAND_BLOCKADDR_MASK (NAND256W3A_BLOCKSIZE - 1)
#define NAND_RDA_CMD 0x00
#define NAND_RDB_CMD 0x01
#define NAND_RDC_CMD 0x50
#define NAND_RDSIGN_CMD 0x90
#define NAND_RDSTATUS_CMD 0x70
#define NAND_PAGEPROG1_CMD 0x80
#define NAND_PAGEPROG2_CMD 0x10
#define NAND_CPBPROG1_CMD 0x00
#define NAND_CPBPROG2_CMD 0x8A
#define NAND_CPBPROG3_CMD 0x10
#define NAND_BLOCKERASE1_CMD 0x60
#define NAND_BLOCKERASE2_CMD 0xD0
#define NAND_RST_CMD 0xFF

#define NAND_STATUS_SR7 0x80
#define NAND_STATUS_SR6 0x40
#define NAND_STATUS_SR0 0x01

#define NAND_SPARE_BADBLOCK_POS 5 /**< Spare area position of bad-block marker. */
#define NAND_SPARE_ECC0_POS 6     /**< Spare area position of ECC byte 0 (LSB). */
#define NAND_SPARE_ECC1_POS 7     /**< Spare area position of ECC byte 1.       */
#define NAND_SPARE_ECC2_POS 8     /**< Spare area position of ECC byte 2 (MSB). */

#define NAND256W3A_SPARESIZE 16

typedef struct
{
    uint32_t baseAddress;                /**< The device base address in cpu memory map.   */
    uint8_t manufacturerCode;            /**< The device manufacturer code.                */
    uint8_t deviceCode;                  /**< The device ID .                              */
    uint32_t deviceSize;                 /**< Total device size in bytes.                  */
    uint32_t pageSize;                   /**< Device page size in bytes.                   */
    uint32_t spareSize;                  /**< Device page spare size in bytes.             */
    uint32_t blockSize;                  /**< Device block size in bytes.                  */
    uint32_t ecc;                        /**< Result of ECC generation from last read/written page. */
    uint8_t spare[NAND256W3A_SPARESIZE]; /**< Spare area content from last read page or spare operation. */
} NANDFLASH_Info_TypeDef;

typedef enum {
    NANDFLASH_STATUS_OK = 0,          /**< No errors detected.                      */
    NANDFLASH_INVALID_DEVICE = -1,    /**< Invalid (unsupported) flash device.      */
    NANDFLASH_INVALID_ADDRESS = -2,   /**< Invalid nand flash address.              */
    NANDFLASH_WRITE_ERROR = -3,       /**< Nand flash write error, block is "bad".  */
    NANDFLASH_ECC_ERROR = -4,         /**< Illegal ECC value read from spare area.  */
    NANDFLASH_ECC_UNCORRECTABLE = -5, /**< Uncorrectable data error in page.        */
    NANDFLASH_INVALID_SETUP = -6,     /**< Invalid parameter to NANDFLASH_Init().   */
    NANDFLASH_NOT_INITIALIZED = -7,   /**< Nand module has not been initialized.    */
} NANDFLASH_Status_TypeDef;


void DoThings(FlashInterface* flash)
{
	LOG(LOG_LEVEL_DEBUG, "Initializing flash");

	flash->initialize(flash);

	FlashStatus status = flash->check(flash);

	uint8_t counterPos = 0;

	if(status != FlashStatusOK)
	{
		LOGF(LOG_LEVEL_ERROR, "Flash check failed: %d", status);
	}
	else
	{
		LOG(LOG_LEVEL_INFO, "Flash initialized success");
	}

	uint8_t buffer[512] = {0};

	flash->readPage(flash, flash->baseAddress, buffer);

	LOGF(LOG_LEVEL_INFO, "Boot counter: %d", buffer[counterPos]);

	if(buffer[counterPos] == 255)
	{
		LOG(LOG_LEVEL_DEBUG, "Boot counter uninitalized");
		buffer[counterPos] = 0;
	}
	else
	{
		LOG(LOG_LEVEL_DEBUG, "Incrementing boot counter");
		buffer[counterPos]++;
	}

	flash->writePage(flash, flash->baseAddress, buffer);

	flash->readPage(flash, flash->baseAddress, buffer);
}
