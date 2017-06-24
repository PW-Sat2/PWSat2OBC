#include "bsp/bsp_ebi.h"
#include "bsp/bsp_uart.h"

static void checkSram(size_t i, char value, size_t* errors)
{
    *(volatile uint8_t*)(BSP_EBI_SRAM1_BASE + i) = value;
    char r = *(volatile uint8_t*)(BSP_EBI_SRAM1_BASE + i);

    if (r != value)
    {
        BSP_UART_Printf<80>(BSP_UART_DEBUG, "%.6X (%.2X -> %.2X)\n", i, value, r);
        (*errors)++;
    }
}

static void testSram_FF()
{
    size_t size = 1000; // 2 * 1024 * 1024;
    size_t errors = 0;

    BSP_UART_Puts(BSP_UART_DEBUG, "\ntestSram_FF ");

    auto Sram = (volatile uint8_t*)BSP_EBI_SRAM1_BASE;

    for (size_t i = 0; i < size; i++)
    {
        Sram[i] = 0x55;
    }
    Delay(10);
    for (size_t i = 0; i < size; i++)
    {
        uint8_t r = Sram[i];

        if (r != 0x55)
        {
            BSP_UART_Printf<80>(BSP_UART_DEBUG, "%.6X (%.2X -> %.2X)\n", i, 0x55, r);

            errors++;
        }
    }

    if (errors == 0)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "------------OK\n");
    }
    else
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "------------ER\n");
    }
}

static void testSram_M()
{
    size_t size = 2 * 1024 * 1024;
    size_t errors = 0;

    BSP_UART_Puts(BSP_UART_DEBUG, "\ntestSram_M ");

    auto Sram = (volatile uint8_t*)BSP_EBI_SRAM1_BASE;

    for (size_t i = 0; i < size; i++)
    {
        uint8_t expected = i % 256;
        i[Sram] = expected;
    }

    Delay(10);
    for (size_t i = 0; i < size; i++)
    {
        uint8_t expected = i % 256;
        uint8_t now = Sram[i];

        if (now != expected)
        {
            BSP_UART_Printf<80>(BSP_UART_DEBUG, "%.6X (%.2X -> %.2X)\n", i, expected, now);

            errors++;
        }
    }

    if (errors == 0)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "------------OK\n");
    }
    else
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "------------ER\n");
    }
}

static void SRAMTestModulo(void)
{
    size_t size = 2 * 1024 * 1024;
    size_t errors = 0;

    BSP_UART_Puts(BSP_UART_DEBUG, "\ntestSram ");

    for (size_t i = 0; i < size; i++)
    {
        char value = i % 256;
        checkSram(i, 0xFF, &errors);
        checkSram(i, value, &errors);
    }

    if (errors == 0)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "------------OK\n");
    }
    else
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "------------ER\n");
    }
}

void checkEeprom(size_t i, char value, size_t* errors)
{
    char buf[1] = {0};
    buf[0] = value;

    BSP_EBI_progEEPROM(i, (uint8_t*)buf, 1);

    char r = *(volatile uint8_t*)(BSP_EBI_EEPROM_BASE + i);

    if (r != value)
    {
        BSP_UART_Printf<80>(BSP_UART_DEBUG, "%.6X (%.2X -> %.2X)\n", i, value, r);
        (*errors)++;
    }
}

void Test()
{
    BSP_UART_txByte(BSP_UART_DEBUG, 'X');
}

void TestSRAM()
{
    while (1)
    {
        testSram_M();
        testSram_FF();
        SRAMTestModulo();
    }
}

void TestEEPROM()
{
    while (1)
    {
        size_t size = 32 * 1024;
        size_t errors = 0;
        size_t progress = 0;

        BSP_UART_Puts(BSP_UART_DEBUG, "\ntestEeprom ");

        for (size_t i = 0; i < size; i++)
        {
            char value = i % 256;
            checkEeprom(i, 0xFF, &errors);
            checkEeprom(i, value, &errors);

            if (i % 256 == 0)
            {
                progress = (i * 100) / size;
                BSP_UART_Printf<80>(BSP_UART_DEBUG, "\nAddr. range: %.6X-%.6X (%u%% done)", i, i + 255, progress);
            }
        }

        if (errors == 0)
        {
            BSP_UART_Puts(BSP_UART_DEBUG, "------------OK\n");
        }
        else
        {
            BSP_UART_Puts(BSP_UART_DEBUG, "------------ER\n");
        }
    }
}

void Recovery()
{
    BSP_UART_Printf<50>(BSP_UART_DEBUG,
        "\nSRAM1: %s SRAM2: %s\n",
        BSP_SEE_isSramLatched(bspSeeSram1) ? "Latched" : "Not latched",
        BSP_SEE_isSramLatched(bspSeeSram2) ? "Latched" : "Not latched");

    if (BSP_SEE_isSramLatched(bspSeeSram1))
    {
        BSP_SEE_tryLatchupRecovery(bspSeeSram1, 5);
    };

    if (BSP_SEE_isSramLatched(bspSeeSram2))
    {
        BSP_SEE_tryLatchupRecovery(bspSeeSram2, 5);
    };

    BSP_UART_Printf<50>(BSP_UART_DEBUG,
        "\nSRAM1: %s SRAM2: %s\n",
        BSP_SEE_isSramLatched(bspSeeSram1) ? "Latched" : "Not latched",
        BSP_SEE_isSramLatched(bspSeeSram2) ? "Latched" : "Not latched");
}
