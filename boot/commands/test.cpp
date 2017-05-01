#include "bsp/bsp_ebi.h"
#include "bsp/bsp_uart.h"

static void checkSram(size_t i, char value, size_t* errors)
{
    *(volatile uint8_t*)(BSP_EBI_SRAM1_BASE + i) = value;
    char r = *(volatile uint8_t*)(BSP_EBI_SRAM1_BASE + i);

    if (r != value)
    {
        char buf[80] = {0};
        sprintf(buf, "%.6X (%.2X -> %.2X)\n", i, value, r);
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)buf, sizeof(buf), true);
        (*errors)++;
    }
}

static void testSram_FF()
{
    size_t size = 1000; // 2 * 1024 * 1024;
    size_t errors = 0;

    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)"\ntestSram_FF ", 13, true);

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
            char buf[80] = {0};
            sprintf(buf, "%.6X (%.2X -> %.2X)\n", i, 0x55, r);
            BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)buf, sizeof(buf), true);

            errors++;
        }
    }

    if (errors == 0)
    {
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)"------------OK\n", 16, true);
    }
    else
    {
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)"------------ER\n", 16, true);
    }
}

static void testSram_M()
{
    size_t size = 2 * 1024 * 1024;
    size_t errors = 0;

    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)"\ntestSram_M ", 12, true);

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
            char buf[80] = {0};
            sprintf(buf, "%.6X (%.2X -> %.2X)\n", i, expected, now);
            BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)buf, sizeof(buf), true);

            errors++;
        }
    }

    if (errors == 0)
    {
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)"------------OK\n", 16, true);
    }
    else
    {
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)"------------ER\n", 16, true);
    }
}

static void SRAMTestModulo(void)
{
    size_t size = 2 * 1024 * 1024;
    size_t errors = 0;

    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)"\ntestSram ", 10, true);

    for (size_t i = 0; i < size; i++)
    {
        char value = i % 256;
        checkSram(i, 0xFF, &errors);
        checkSram(i, value, &errors);
    }

    if (errors == 0)
    {
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)"------------OK\n", 16, true);
    }
    else
    {
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)"Err\n", 4, true);
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
        char buf[80] = {0};
        sprintf(buf, "%.6X (%.2X -> %.2X)\n", i, value, r);
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)buf, sizeof(buf), true);
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
        char buf[50] = {0};

        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)"\ntestEeprom ", 12, true);

        for (size_t i = 0; i < size; i++)
        {
            char value = i % 256;
            checkEeprom(i, 0xFF, &errors);
            checkEeprom(i, value, &errors);

            if (i % 256 == 0)
            {
                progress = (i * 100) / size;
                sprintf(buf, "\nAddr. range: %.6X-%.6X (%u%% done)", i, i + 255, progress);
                BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)buf, sizeof(buf), true);
            }
        }

        if (errors == 0)
        {
            BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)"\n------------OK\n", 17, true);
        }
        else
        {
            BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)"\nErr\n", 5, true);
        }
    }
}
