#include <stdio.h>
#include <string.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_emu.h>
#include <em_usart.h>
#include "mcu/io_map.h"
#include "system.h"

#include "boot/params.hpp"

static void SendToUart(USART_TypeDef* uart, const char* message)
{
    while (*message != '\0')
    {
        USART_Tx(uart, *message);
        message++;
    }
}

extern "C" void __libc_init_array(void);

int main()
{
    SCB->VTOR = 0x00080000;

    CHIP_Init();

    __libc_init_array();

    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(cmuClock_DMA, true);

    USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
    init.baudrate = io_map::UART_1::Baudrate;
    init.enable = usartDisable;

    CMU_ClockEnable(cmuClock_UART1, true);
    USART_InitAsync(io_map::UART_1::Peripheral, &init);

    io_map::UART_1::Peripheral->ROUTE |= UART_ROUTE_TXPEN | UART_ROUTE_RXPEN | io_map::UART_1::Location;

    USART_Enable(io_map::UART_1::Peripheral, usartEnable);

    USART_Tx(io_map::UART_1::Peripheral, '!');

    char msg[256] = {0};

    sprintf(msg, "Magic: 0x%lX\nReason=%d\nIndex=%d\n", boot::MagicNumber, num(boot::BootReason), boot::Index);

    SendToUart(io_map::UART_1::Peripheral, msg);

    SendToUart(io_map::UART_1::Peripheral, "Sleeping!\n");
    EMU_EnterEM2(true);

    SendToUart(io_map::UART_1::Peripheral, "Wake up!\n");

    while (1)
        ;
}