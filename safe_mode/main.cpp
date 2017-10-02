#include <stdio.h>
#include <string.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_emu.h>
#include <em_usart.h>

#include "boot/params.hpp"
#include "logger/logger.h"
#include "mcu/io_map.h"
#include "system.h"

#include "safe_mode.hpp"
#include "steps/erase_n25q/step.hpp"
#include "steps/reboot/step.hpp"
#include "steps/revert_boot_slots/step.hpp"
#include "steps/scrub_bootloader/step.hpp"
#include "steps/scrub_program/step.hpp"
#include "steps/steps.hpp"

#include "version.h"

OBCSafeMode SafeMode;

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

extern "C" void assertFailed(const char* /*source*/, const char* /*file*/, std::uint16_t /*line*/)
{
    while (1)
        ;
}

void SysTick_Handler()
{
    USART_Tx(io_map::UART_1::Peripheral, '*');
}

static void Recover()
{
    RecoverySteps<ScrubBootloader, ScrubProgram, EraseN25QStep, RevertBootSlots, RebootStep> steps;

    steps.Perform();
}

static void SendToUart(USART_TypeDef* uart, const char* message)
{
    while (*message != '\0')
    {
        USART_Tx(uart, *message);
        message++;
    }
}

static void LogToUart(void* context, bool /*withinIsr*/, const char* messageHeader, const char* messageFormat, va_list messageArguments)
{
    char buf[256];
    auto uart = static_cast<USART_TypeDef*>(context);
    SendToUart(uart, messageHeader);
    vsprintf(buf, messageFormat, messageArguments);
    SendToUart(uart, buf);
    USART_Tx(uart, '\n');
}

void LogVersion(USART_TypeDef* uart)
{
    SendToUart(uart, "Safe Mode " VERSION);
    USART_Tx(uart, '\n');
}

int main(void)
{
    SCB->VTOR = 0x12000000;

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

    LogInit(LOG_LEVEL_DEBUG);
    LogAddEndpoint(LogToUart, io_map::UART_1::Peripheral, LOG_LEVEL_DEBUG);

    char msg[256] = {0};

    sprintf(msg, "Magic: 0x%lX\nReason=%d\nIndex=%d\n", boot::MagicNumber, num(boot::BootReason), boot::Index);

    SendToUart(io_map::UART_1::Peripheral, msg);
    LogVersion(io_map::UART_1::Peripheral);
    Recover();

    SysTick_Config(SystemCoreClockGet());
    NVIC_EnableIRQ(IRQn_Type::SysTick_IRQn);

    while (1)
    {
        auto r = USART_Rx(io_map::UART_1::Peripheral);
        if (r == 'r')
        {
            NVIC_SystemReset();
        }
    }

    return 0;
}
