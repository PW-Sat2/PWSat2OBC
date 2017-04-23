#include <stdio.h>
#include <string.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_emu.h>
#include <em_usart.h>

#include "io_map.h"
#include "system.h"

extern "C" void __libc_init_array(void);

/**
 * @brief emlib-specific assertion failed handler
 * @param file Path to source file with failed assertion
 * @param line Line number with failed assertion
 */
extern "C" void assertEFM(const char* /*file*/, int /*line*/)
{
    while (1)
        ;
}

int main(void)
{
    SCB->VTOR = 0x12000000;

    CHIP_Init();

    __libc_init_array();

    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(cmuClock_DMA, true);

    USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
    init.baudrate = io_map::UART::Baudrate;
    init.enable = usartDisable;

    CMU_ClockEnable(cmuClock_UART1, true);
    USART_InitAsync(io_map::UART::Peripheral, &init);

    io_map::UART::Peripheral->ROUTE |= UART_ROUTE_TXPEN | UART_ROUTE_RXPEN | io_map::UART::Location;

    USART_Enable(io_map::UART::Peripheral, usartEnable);

    USART_Tx(io_map::UART::Peripheral, '!');

    while (1)
        ;

    return 0;
}
