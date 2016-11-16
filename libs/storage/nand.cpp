#include <em_cmu.h>
#include <em_ebi.h>
#include <em_gpio.h>

#include "system.h"

#include "base/ecc.h"
#include "io_map.h"
#include "logger/logger.h"
#include "nand.h"

#define NAND256W3A_SIGNATURE 0x7520

#define NAND256W3A_PAGESIZE 512
#define NAND256W3A_BLOCKSIZE (16 * 1024)
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

#define NAND_STATUS_SR0 0x01

static void ChipEnable(bool enable)
{
    if (enable)
    {
        GPIO_PinOutClear(NAND_CE_PORT, NAND_CE_PIN);
    }
    else
    {
        GPIO_PinOutSet(NAND_CE_PORT, NAND_CE_PIN);
    }
}

static void PowerEnable(bool enable)
{
    if (enable)
    {
        GPIO_PinOutSet(NAND_POWER_PORT, NAND_POWER_PIN);
    }
    else
    {
        GPIO_PinOutClear(NAND_POWER_PORT, NAND_POWER_PIN);
    }
}

static void WaitReady(void)
{
    /* Wait for EBI idle in case of EBI writeBuffer is enabled */
    while (EBI->STATUS & EBI_STATUS_AHBACT)
    {
    }
    /* Wait on Ready/Busy pin to become high */
    while (GPIO_PinInGet(NAND_READY_PORT, NAND_READY_PIN) == 0)
    {
    }
}

static void WriteProtect(bool enable)
{
    if (enable)
    {
        GPIO_PinOutClear(NAND_WP_PORT, NAND_WP_PIN);
    }
    else
    {
        GPIO_PinOutSet(NAND_WP_PORT, NAND_WP_PIN);
    }
}

static void Reset(FlashNANDInterface* nand)
{
    ChipEnable(true);
    WaitReady();
    *(nand->cmd) = NAND_RST_CMD;
    WaitReady();
    ChipEnable(false);
}

static inline void WaitEBI(void)
{
    while (EBI->STATUS & EBI_STATUS_AHBACT)
    {
    }
}

static void EnableEBI(void)
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
    GPIO_PinModeSet(EBI_ALE_PORT, EBI_ALE_PIN, gpioModePushPull, 0);
    GPIO_PinModeSet(EBI_CLE_PORT, EBI_CLE_PIN, gpioModePushPull, 0);

    /* WP, CE and R/B */
    GPIO_PinModeSet(NAND_WP_PORT, NAND_WP_PIN, gpioModePushPull, 0);    /* active low write-protect */
    GPIO_PinModeSet(NAND_CE_PORT, NAND_CE_PIN, gpioModePushPull, 1);    /* active low chip-enable */
    GPIO_PinModeSet(NAND_READY_PORT, NAND_READY_PIN, gpioModeInput, 0); /* ready/busy */

    /* IO pins */
    GPIO_PinModeSet(EBI_DATA_PORT, EBI_DATA_PIN0, gpioModePushPull, 0);
    GPIO_PinModeSet(EBI_DATA_PORT, EBI_DATA_PIN0 + 1, gpioModePushPull, 0);
    GPIO_PinModeSet(EBI_DATA_PORT, EBI_DATA_PIN0 + 2, gpioModePushPull, 0);
    GPIO_PinModeSet(EBI_DATA_PORT, EBI_DATA_PIN0 + 3, gpioModePushPull, 0);
    GPIO_PinModeSet(EBI_DATA_PORT, EBI_DATA_PIN0 + 4, gpioModePushPull, 0);
    GPIO_PinModeSet(EBI_DATA_PORT, EBI_DATA_PIN0 + 5, gpioModePushPull, 0);
    GPIO_PinModeSet(EBI_DATA_PORT, EBI_DATA_PIN0 + 6, gpioModePushPull, 0);
    GPIO_PinModeSet(EBI_DATA_PORT, EBI_DATA_PIN0 + 7, gpioModePushPull, 0);

    /* WE and RE */
    GPIO_PinModeSet(EBI_WE_PORT, EBI_WE_PIN, gpioModePushPull, 1);
    GPIO_PinModeSet(EBI_RE_PORT, EBI_RE_PIN, gpioModePushPull, 1);

    /* NAND Power Enable */
    GPIO_PinModeSet(NAND_POWER_PORT, NAND_POWER_PIN, gpioModePushPull, 1);

    EBI_Init(&ebiConfig);
    EBI->NANDCTRL = (EBI_NANDCTRL_BANKSEL_BANK0 | EBI_NANDCTRL_EN);
}

static FlashStatus initialize(FlashNANDInterface* flash)
{
    EnableEBI();

    uint32_t baseAddress = EBI_BankAddress(EBI_BANK0);

    flash->data8 = (uint8_t volatile*)baseAddress;
    flash->data16 = (uint16_t volatile*)baseAddress;
    flash->data32 = (uint32_t volatile*)baseAddress;
    flash->addr = (uint8_t volatile*)(baseAddress + (1 << NAND_ALE_BIT));
    flash->cmd = (uint8_t volatile*)(baseAddress + (1 << NAND_CLE_BIT));

    PowerEnable(true);

    Reset(flash);

    return flash->check(flash);
}

static FlashStatus readPage(FlashNANDInterface* flash, uint32_t offset, uint8_t* buffer, uint32_t length)
{
    offset &= ~NAND_PAGEADDR_MASK;

    ChipEnable(true);
    WaitReady();

    *(flash->cmd) = NAND_RDA_CMD;
    *(flash->addr) = (uint8_t)offset;
    *(flash->addr) = (uint8_t)(offset >> 9);
    *(flash->addr) = (uint8_t)(offset >> 17);

    WaitReady();

    EBI_StartNandEccGen();

    __packed uint32_t* p = (__packed uint32_t*)buffer;
    for (uint32_t i = 0; i < length / 4; i++)
    {
        *(p + i) = *(flash->data32);
    }

    uint32_t generatedEcc = EBI_StopNandEccGen();

    *(flash->cmd) = NAND_RDC_CMD;
    *(flash->addr) = (uint8_t)((uint8_t)(offset & 0xFF) + 13);
    *(flash->addr) = (uint8_t)(offset >> 9);
    *(flash->addr) = (uint8_t)(offset >> 17);

    while (EBI->STATUS & EBI_STATUS_AHBACT)
    {
    }

    WaitReady();

    uint32_t readEcc = *(flash->data8);
    readEcc |= (*(flash->data8) << 8);
    readEcc |= (*(flash->data8) << 16);

    ChipEnable(false);

    if (readEcc == 0x00FFFFFF)
    {
        return FlashStatusOK;
    }

    EccResult correction = EccCorrect(generatedEcc, readEcc, buffer, length);

    switch (correction)
    {
        case EccResultCorrected:
            return FlashStatusErrorCorrected;
        case EccResultNotCorrected:
            return FlashStatusErrorNotCorrected;
        case EccResultCorrupted:
            return FlashStatusChecksumCorrupted;
        case EccResultNoError:
        default:
            return FlashStatusOK;
    }
}

static FlashStatus writePage(FlashNANDInterface* flash, uint32_t offset, uint8_t* const buffer, uint32_t length)
{
    offset = offset & ~NAND_PAGEADDR_MASK;

    WriteProtect(false);
    ChipEnable(true);

    *(flash->cmd) = NAND_RDA_CMD;
    *(flash->cmd) = NAND_PAGEPROG1_CMD;
    *(flash->addr) = (uint8_t)offset;
    *(flash->addr) = (uint8_t)(offset >> 9);
    *(flash->addr) = (uint8_t)(offset >> 17);

    WaitEBI();

    WaitReady();

    EBI_StartNandEccGen();

    __packed uint32_t* const p = (__packed uint32_t * const)buffer;
    for (uint32_t i = 0; i < length / 4; i++)
    {
        *(flash->data32) = *(p + i);
    }

    WaitEBI();

    uint32_t ecc = EBI_StopNandEccGen();

    *(flash->cmd) = NAND_PAGEPROG2_CMD;

    WaitReady();

    int flashStatus = flash->status(flash);
    FlashStatus status = (flashStatus & NAND_STATUS_SR0) ? FlashStatusWriteError : FlashStatusOK;

    if (status != FlashStatusOK)
    {
        ChipEnable(false);
        WriteProtect(true);
        return status;
    }

    *(flash->cmd) = NAND_RDC_CMD;
    *(flash->cmd) = NAND_PAGEPROG1_CMD;
    *(flash->addr) = (uint8_t)((uint8_t)(offset & 0xFF) + 13);
    *(flash->addr) = (uint8_t)(offset >> 9);
    *(flash->addr) = (uint8_t)(offset >> 17);

    WaitEBI();

    WaitReady();

    *(flash->data8) = (uint8_t)(ecc & 0xFF);
    *(flash->data8) = (uint8_t)((ecc >> 8) & 0xFF);
    *(flash->data8) = (uint8_t)((ecc >> 16) & 0xFF);

    *(flash->cmd) = NAND_PAGEPROG2_CMD;

    WaitReady();

    flashStatus = flash->status(flash);
    status = (flashStatus & NAND_STATUS_SR0) ? FlashStatusWriteError : FlashStatusOK;

    ChipEnable(false);
    WriteProtect(true);

    return status;
}

static FlashStatus eraseBlock(FlashNANDInterface* flash, uint32_t address)
{
    int status;

    address &= ~NAND_BLOCKADDR_MASK;

    WriteProtect(false);
    ChipEnable(true);

    *(flash->cmd) = NAND_BLOCKERASE1_CMD;
    /* Coloumn address, bit 8 is not used, implicitely defined by NAND_RDA_CMD. */
    *(flash->addr) = (uint8_t)(address >> 9);
    *(flash->addr) = (uint8_t)(address >> 17);
    *(flash->cmd) = NAND_BLOCKERASE2_CMD;

    WaitReady();

    status = (flash->status(flash) & NAND_STATUS_SR0) ? FlashStatusWriteError : FlashStatusOK;

    ChipEnable(false);
    WriteProtect(true);

    return status;
}

static FlashStatus check(FlashNANDInterface* flash)
{
    ChipEnable(true);

    *(flash->cmd) = NAND_RDSIGN_CMD;
    uint16_t sig = *(flash->data16);

    ChipEnable(false);

    if (sig != NAND256W3A_SIGNATURE)
    {
        EFM_ASSERT(false);
        return FlashStatusInvalidDevice;
    }

    return FlashStatusOK;
}

static uint32_t status(FlashNANDInterface* flash)
{
    *(flash->cmd) = NAND_RDSTATUS_CMD;
    return *(flash->data8);
}

static bool isBadBlock(FlashNANDInterface* flash, uint32_t address)
{
    address &= ~NAND_PAGEADDR_MASK;

    ChipEnable(true);
    WaitReady();

    *(flash->cmd) = NAND_RDC_CMD;
    *(flash->addr) = (uint8_t)((uint8_t)(address & 0xFF) + 5);
    *(flash->addr) = (uint8_t)(address >> 9);
    *(flash->addr) = (uint8_t)(address >> 17);

    WaitReady();

    uint8_t badBlockMark = *(flash->data8);

    ChipEnable(false);

    return badBlockMark != 0xFF;
}

static FlashStatus markBadBlock(FlashNANDInterface* flash, uint32_t offset)
{
    offset &= ~NAND_BLOCKADDR_MASK;

    WriteProtect(false);
    ChipEnable(true);

    *(flash->cmd) = NAND_RDC_CMD;
    *(flash->cmd) = NAND_PAGEPROG1_CMD;
    *(flash->addr) = (uint8_t)offset;
    /* Address bit 8 is not used, implicitely defined by NAND_RDC_CMD. */
    *(flash->addr) = (uint8_t)(offset >> 9);
    *(flash->addr) = (uint8_t)(offset >> 17);

    /* Write bad block marker 0x00 to the 6th byte in the spare area */
    *(flash->data32) = 0xFFFFFFFF;
    *(flash->data16) = 0x00FF;
    *(flash->cmd) = NAND_PAGEPROG2_CMD;

    WaitReady();

    FlashStatus status = (flash->status(flash) & NAND_STATUS_SR0) ? FlashStatusWriteError : FlashStatusOK;

    ChipEnable(false);
    WriteProtect(true);

    return status;
}

void BuildNANDInterface(FlashNANDInterface* flash)
{
    flash->initialize = initialize;
    flash->readPage = readPage;
    flash->writePage = writePage;
    flash->check = check;
    flash->status = status;
    flash->eraseBlock = eraseBlock;
    flash->isBadBlock = isBadBlock;
    flash->markBadBlock = markBadBlock;
}
