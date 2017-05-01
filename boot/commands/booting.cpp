#include <cstdint>
#include "boot.h"
#include "bsp/bsp_boot.h"
#include "bsp/bsp_uart.h"

using std::uint8_t;

void BootUpper()
{
    char debugStr[60] = {0};
    auto debugLen = sprintf((char*)debugStr, "\n\nBooting to upper half ");
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

    BootToAddress(BOOT_APPLICATION_BASE);
}

void BootToSelected()
{
    char debugStr[60] = {0};

    auto index = USART_Rx(BSP_UART_DEBUG);
    if (index > BOOT_TABLE_SIZE)
    {
        auto debugLen = sprintf((char*)debugStr, "\n\nError: Boot index out of bounds!");
        BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);
        return;
    }

    auto debugLen = sprintf((char*)debugStr, "\nBooting index: %d", index);
    BSP_UART_txBuffer(BSP_UART_DEBUG, (uint8_t*)debugStr, debugLen, true);

    BootToAddress(LoadApplication(index));
}
