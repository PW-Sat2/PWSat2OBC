#include "mcu/io_map.h"

namespace io_map
{
    USART_TypeDef* const SPI::Peripheral = USART0;
    USART_TypeDef* const UART_0::Peripheral = UART0;
    USART_TypeDef* const UART_1::Peripheral = UART1;
    std::uint8_t* const ProgramFlash::FlashBase = reinterpret_cast<std::uint8_t*>(0x84000000);
    std::uint8_t* const ProgramFlash::ApplicatonBase = reinterpret_cast<std::uint8_t*>(0x00080000);
    TIMER_TypeDef* const RAMScrubbing::TimerHW = TIMER0;
    ACMP_TypeDef* const BSP::Latchup::HW = ACMP0;
    ACMP_TypeDef* const MemoryModule<1>::Comparator = ACMP0;
    ACMP_TypeDef* const MemoryModule<2>::Comparator = ACMP1;
}
