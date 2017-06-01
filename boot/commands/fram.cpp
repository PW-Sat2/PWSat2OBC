#include <array>
#include <cstdint>
#include <em_cmu.h>
#include <em_usart.h>
#include <gsl/span>
#include "bsp/bsp_uart.h"
#include "fm25w/fm25w.hpp"
#include "gpio/gpio.h"
#include "io_map.h"
#include "spi.hpp"
#include "system.h"

using namespace devices::fm25w;
using gsl::span;
using std::uint8_t;

void TestFRAM()
{
    BSP_UART_Puts(BSP_UART_DEBUG, "FRAMs!\n");

    SPIPeripheral spi;

    spi.Initialize();

    SPIDevice<io_map::SlaveSelectFram1> spi1(spi);
    SPIDevice<io_map::SlaveSelectFram2> spi2(spi);
    SPIDevice<io_map::SlaveSelectFram3> spi3(spi);

    spi1.Initialize();
    spi2.Initialize();
    spi3.Initialize();

    FM25WDriver fram1(spi1);
    FM25WDriver fram2(spi2);
    FM25WDriver fram3(spi3);

    RedundantFM25WDriver fram({&fram1, &fram2, &fram3});

    auto status = fram.ReadStatus();

    if (status.HasValue)
    {
        BSP_UART_Printf<20>(BSP_UART_DEBUG, "Status = %d\n", num(status.Value));
    }
    else
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "Fail!\n");
    }

    std::array<uint8_t, 4> data{0xA, 0xB, 0xC, 0xD};
    std::array<uint8_t, 4> readBack{0xF, 0xF, 0xF, 0xF};

    fram.Write(2, data);
    fram.Read(2, readBack);

    BSP_UART_Printf<20>(BSP_UART_DEBUG, "Read back: %X %X %X %X", readBack[0], readBack[1], readBack[2], readBack[3]);

    spi.Deinitialize();
}
