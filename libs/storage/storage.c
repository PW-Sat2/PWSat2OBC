#include <em_cmu.h>
#include <em_ebi.h>
#include <em_gpio.h>

#include "Logger/logger.h"

#define NAND_POWER_PORT gpioPortB
#define NAND_POWER_PIN (1 << 15)
#define NAND_READY_PORT gpioPortD
#define NAND_READY_PIN (1 << 15)
#define NAND_CE_PORT gpioPortD
#define NAND_CE_PIN (1 << 14)
#define NAND_WP_PORT gpioPortD
#define NAND_WP_PIN (1 << 13)
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

static NANDFLASH_Info_TypeDef flashInfo;
static bool flashInitialized = false;
static uint8_t volatile* pNandData8;
static uint16_t volatile* pNandData16;
static uint32_t volatile* pNandData32;
static uint8_t volatile* pNandAddr;
static uint8_t volatile* pNandCmd;

void ChipEnable(bool enable)
{
    if (enable)
    {
        GPIO->P[NAND_CE_PORT].DOUTCLR = NAND_CE_PIN;
    }
    else
    {
        GPIO->P[NAND_CE_PORT].DOUTSET = NAND_CE_PIN;
    }
}

static void PowerEnable(bool enable)
{
    if (enable)
    {
        GPIO->P[NAND_POWER_PORT].DOUTSET = NAND_POWER_PIN;
    }
    else
    {
        GPIO->P[NAND_POWER_PORT].DOUTCLR = NAND_POWER_PIN;
    }
}

static void WaitReady(void)
{
    /* Wait for EBI idle in case of EBI writeBuffer is enabled */
    while (EBI->STATUS & EBI_STATUS_AHBACT)
    {
    }
    /* Wait on Ready/Busy pin to become high */
    while ((GPIO->P[NAND_READY_PORT].DIN & NAND_READY_PIN) == 0)
    {
    }
}

static void Reset(void)
{
    *pNandCmd = NAND_RST_CMD;
    WaitReady();
}

static uint16_t ReadSignature(void)
{
    *pNandCmd = NAND_RDSIGN_CMD;
    uint16_t sig = *pNandData16;

    return sig;
}

static int flashInterrogate(void)
{
    flashInfo.baseAddress = EBI_BankAddress(EBI_BANK0);

    pNandData8 = (uint8_t volatile*)flashInfo.baseAddress;
    pNandData16 = (uint16_t volatile*)pNandData8;
    pNandData32 = (uint32_t volatile*)pNandData8;
    pNandAddr = pNandData8 + (1 << NAND_ALE_BIT);
    pNandCmd = pNandData8 + (1 << NAND_CLE_BIT);

    PowerEnable(true);
    WaitReady();
    ChipEnable(true);
    Reset();

    if (ReadSignature() != NAND256W3A_SIGNATURE)
    {
        EFM_ASSERT(false);
        return NANDFLASH_INVALID_DEVICE;
    }

    ChipEnable(false);

    flashInfo.manufacturerCode = (uint8_t)NAND256W3A_SIGNATURE;
    flashInfo.deviceCode = (uint8_t)(NAND256W3A_SIGNATURE >> 8);
    flashInfo.deviceSize = NAND256W3A_SIZE;
    flashInfo.pageSize = NAND256W3A_PAGESIZE;
    flashInfo.spareSize = NAND256W3A_SPARESIZE;
    flashInfo.blockSize = NAND256W3A_BLOCKSIZE;

    flashInitialized = true;

    return NANDFLASH_STATUS_OK;
}

void NAND_Init(void)
{
}

void EBIBankInit(void)
{
    EBI_Init_TypeDef ebiConfig = {
        ebiModeD8A8,  /* 8 bit address, 8 bit data */
        ebiActiveLow, /* ARDY polarity */
        ebiActiveLow, /* ALE polarity */
        ebiActiveLow, /* WE polarity */
        ebiActiveLow, /* RE polarity */
        ebiActiveLow, /* CS polarity */
        ebiActiveLow, /* BL polarity */
        false,        /* disble BL */
        true,         /* enable NOIDLE */
        false,        /* disable ARDY */
        true,         /* disable ARDY timeout */
        EBI_BANK0,    /* enable bank 0 */
        0,            /* no chip select */
        0,            /* addr setup cycles */
        0,            /* addr hold cycles */
        false,        /* disable half cycle ALE strobe */
        0,            /* read setup cycles */
        2,            /* read strobe cycles */
        1,            /* read hold cycles */
        false,        /* disable page mode */
        false,        /* disable prefetch */
        false,        /* disable half cycle REn strobe */
        0,            /* write setup cycles */
        2,            /* write strobe cycles */
        1,            /* write hold cycles */
        false,        /* enable the write buffer */
        false,        /* disable half cycle WEn strobe */
        ebiALowA24,   /* ALB - Low bound, address lines */
        ebiAHighA26,  /* APEN - High bound, address lines */
        ebiLocation1, /* Use Location 1 */
        true,         /* enable EBI */
    };

    /* Enable clocks */
    CMU_ClockEnable(cmuClock_EBI, true);

    /* Enable GPIO's */
    /* ALE and CLE */
    GPIO_PinModeSet(gpioPortC, 1, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortC, 2, gpioModePushPull, 0);

    /* WP, CE and R/B */
    GPIO_PinModeSet(gpioPortD, 13, gpioModePushPull, 0); /* active low write-protect */
    GPIO_PinModeSet(gpioPortD, 14, gpioModePushPull, 1); /* active low chip-enable */
    GPIO_PinModeSet(gpioPortD, 15, gpioModeInput, 0);    /* ready/busy */

    /* IO pins */
    GPIO_PinModeSet(gpioPortE, 8, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 9, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 10, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 11, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 12, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 13, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 14, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortE, 15, gpioModePushPull, 0);

    /* WE and RE */
    GPIO_PinModeSet(gpioPortF, 8, gpioModePushPull, 1);
    GPIO_PinModeSet(gpioPortF, 9, gpioModePushPull, 1);

    /* NAND Power Enable */
    GPIO_PinModeSet(gpioPortB, 15, gpioModePushPull, 1);

    EBI_Init(&ebiConfig);
    EBI->NANDCTRL = (EBI_NANDCTRL_BANKSEL_BANK0 | EBI_NANDCTRL_EN);
}

void DoThings(void)
{
    EBIBankInit();
    uint8_t r = flashInterrogate();
    LOGF(LOG_LEVEL_INFO, "Flash interrogate result: %d", r);
}
