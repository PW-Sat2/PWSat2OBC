#include <em_cmu.h>
#include <em_ebi.h>
#include <em_gpio.h>

#include "system.h"

#include "nand.h"

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

#define NAND256W3A_PAGESIZE 512
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

void ChipEnable(bool enable)
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

static void enableEBI(void)
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

static int initialize(FlashInterface* flash)
{
    enableEBI();

    flash->baseAddress = EBI_BankAddress(EBI_BANK0);

    FlashNANDInterface* nand = flash->extra;

    nand->data8 = (uint8_t volatile*)flash->baseAddress;
    nand->data16 = (uint16_t volatile*)flash->baseAddress;
    nand->data32 = (uint32_t volatile*)flash->baseAddress;
    nand->addr = (uint8_t volatile*)(flash->baseAddress + (1 << NAND_ALE_BIT));
    nand->cmd = (uint8_t volatile*)(flash->baseAddress + (1 << NAND_CLE_BIT));

    PowerEnable(true);

    Reset(flash->extra);

    return 0;
}

static int readPage(FlashInterface* flash, uint32_t address, uint8_t* buffer)
{
    FlashNANDInterface* nand = flash->extra;

    uint32_t a = (uint32_t)address;

    a &= ~NAND_PAGEADDR_MASK;

    ChipEnable(true);
    WaitReady();

    *(nand->cmd) = NAND_RDA_CMD;
    *(nand->addr) = (uint8_t)a;
    *(nand->addr) = (uint8_t)(a >> 9);
    *(nand->addr) = (uint8_t)(a >> 17);

    WaitReady();

    EBI_StartNandEccGen();

    uint32_t* p = (uint32_t*)buffer;
    for (uint16_t i = 0; i < 512 / 4; i++)
    {
        *(p + i) = *(nand->data32);
    }

    EBI_StopNandEccGen();

    ChipEnable(false);

    return -1;
}

static FlashStatus writePage(FlashInterface* flash, uint8_t volatile* address, uint8_t* buffer)
{
    FlashNANDInterface* nand = flash->extra;

    address = (uint8_t volatile*)((uint32_t)address & ~NAND_PAGEADDR_MASK);

    WriteProtect(false);
    ChipEnable(true);

    *(nand->cmd) = NAND_RDA_CMD;
    *(nand->cmd) = NAND_PAGEPROG1_CMD;
    *(nand->addr) = (uint8_t)(uint32_t)address;
    *(nand->addr) = (uint8_t)((uint32_t)address >> 9);
    *(nand->addr) = (uint8_t)((uint32_t)address >> 17);

    while (EBI->STATUS & EBI_STATUS_AHBACT)
    {
    }

    WaitReady();

    EBI_StartNandEccGen();

    uint32_t* p = (uint32_t*)buffer;
    for (uint16_t i = 0; i < 512 / 4; i++)
    {
        *(nand->data32) = *(p + i);
    }

    while (EBI->STATUS & EBI_STATUS_AHBACT)
    {
    }

    EBI_StopNandEccGen();

    WaitReady();

    FlashStatus status = (flash->status(flash) & NAND_STATUS_SR0) ? FlashStatusWriteError : FlashStatusOK;

    ChipEnable(false);
    WriteProtect(true);

    return status;
}

static FlashStatus eraseBlock(FlashInterface* flash, uint32_t address)
{
}

int check(FlashInterface* flash)
{
    FlashNANDInterface* nand = flash->extra;

    ChipEnable(true);

    *(nand->cmd) = NAND_RDSIGN_CMD;
    uint16_t sig = *(nand->data16);

    ChipEnable(false);

    if (sig != NAND256W3A_SIGNATURE)
    {
        EFM_ASSERT(false);
        return FlashStatusInvalidDevice;
    }

    return FlashStatusOK;
}

int status(FlashInterface* flash)
{
    FlashNANDInterface* nand = flash->extra;
    *(nand->cmd) = NAND_RDSTATUS_CMD;
    return *(nand->data8);
}

void BuildNANDInterface(FlashInterface* flash, FlashNANDInterface* nand)
{
    flash->extra = nand;

    flash->initialize = initialize;
    flash->readPage = readPage;
    flash->writePage = writePage;
    flash->check = check;
    flash->status = status;
}
