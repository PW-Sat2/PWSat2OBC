#include <stdio.h>
#include <string.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_emu.h>
#include <em_usart.h>

#include "boot/params.hpp"
#include "mcu/io_map.h"
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

void SysTick_Handler()
{
    USART_Tx(io_map::UART::Peripheral, '*');
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

    char msg[256] = {0};

    sprintf(msg, "Magic: 0x%lX\nReason=%d\nIndex=%d\n", boot::MagicNumber, num(boot::BootReason), boot::Index);

    char* c = msg;

    while (*c != '\0')
    {
        USART_Tx(io_map::UART::Peripheral, *c);
        c++;
    }

    SysTick_Config(SystemCoreClockGet());
    NVIC_EnableIRQ(IRQn_Type::SysTick_IRQn);

    while (1)
    {
        auto r = USART_Rx(io_map::UART::Peripheral);
        if (r == 'r')
        {
            NVIC_SystemReset();
        }
    }

    return 0;
}
