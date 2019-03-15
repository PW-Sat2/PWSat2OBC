#include <cstdio>
#include <em_system.h>
#include "core_cm3.h"
#include "mcu/io_map.h"

void SendToUart(USART_TypeDef* uart, const char* message);

static void Hang()
{
    for (;;)
        ;
}

#define UNREFERENCED_PARAMETER(x) ((void)x)

extern "C" __attribute__((used)) void prvGetRegistersFromStack(uint32_t* pulFaultStackAddress)
{
    /* These are volatile to try and prevent the compiler/linker optimising them
    away as the variables never actually get used.  If the debugger won't show the
    values of the variables, make them global my moving their declaration outside
    of this function. */

    volatile uint32_t r0 = pulFaultStackAddress[0];
    volatile uint32_t r1 = pulFaultStackAddress[1];
    volatile uint32_t r2 = pulFaultStackAddress[2];
    volatile uint32_t r3 = pulFaultStackAddress[3];

    volatile uint32_t r12 = pulFaultStackAddress[4];
    volatile uint32_t lr = pulFaultStackAddress[5];  /* Link register. */
    volatile uint32_t pc = pulFaultStackAddress[6];  /* Program counter. */
    volatile uint32_t psr = pulFaultStackAddress[7]; /* Program status register. */

    //    volatile uint16_t* nvic = *((uint16_t*)0xe000ed04);
    uint32_t cfsr = SCB->CFSR;
    uint32_t hfsr = SCB->HFSR;
    uint32_t mmfar = SCB->MMFAR;
    uint32_t bfar = SCB->BFAR;
    uint32_t icsr = SCB->ICSR;

    UNREFERENCED_PARAMETER(r0);
    UNREFERENCED_PARAMETER(r1);
    UNREFERENCED_PARAMETER(r2);
    UNREFERENCED_PARAMETER(r3);
    UNREFERENCED_PARAMETER(r12);
    UNREFERENCED_PARAMETER(lr);
    UNREFERENCED_PARAMETER(pc);
    UNREFERENCED_PARAMETER(psr);
    UNREFERENCED_PARAMETER(cfsr);
    UNREFERENCED_PARAMETER(hfsr);
    UNREFERENCED_PARAMETER(mmfar);
    UNREFERENCED_PARAMETER(bfar);

    /* When the following line is hit, the variables contain the register values. */

    char msg[256] = {0};

    sprintf(msg,
        "ICSR:0x%X\nCFSR: 0x%X\nHFSR: 0x%X\nMMFAR: 0x%X\nBFAR: 0x%X\nLR: 0x%X\nPC: 0x%X\nPSR: 0x%X",
        (unsigned int)icsr,
        (unsigned int)cfsr,
        (unsigned int)hfsr,
        (unsigned int)mmfar,
        (unsigned int)bfar,
        (unsigned int)lr,
        (unsigned int)pc,
        (unsigned int)psr);
    SendToUart(io_map::UART_1::Peripheral, msg);

    Hang();
}

extern "C" void CustomDefaultHandler()
{
    __asm volatile(" tst lr, #4                                                \n"
                   " ite eq                                                    \n"
                   " mrseq r0, msp                                             \n"
                   " mrsne r0, psp                                             \n"
                   " ldr r1, [r0, #24]                                         \n"
                   " ldr r2, handler2_address_constHF                          \n"
                   " bx r2                                                     \n"
                   " handler2_address_constHF: .word prvGetRegistersFromStack  \n");

    NVIC_SystemReset();
}

#define HANDLER(name) void name() __attribute__((alias("CustomDefaultHandler")));

HANDLER(NMI_Handler)
HANDLER(HardFault_Handler)
HANDLER(MemManage_Handler)
HANDLER(BusFault_Handler)
HANDLER(UsageFault_Handler)
HANDLER(SVC_Handler)
HANDLER(DebugMon_Handler)
HANDLER(PendSV_Handler)
HANDLER(DMA_IRQHandler)
HANDLER(GPIO_EVEN_IRQHandler)
HANDLER(TIMER0_IRQHandler)
HANDLER(USART0_RX_IRQHandler)
HANDLER(USART0_TX_IRQHandler)
HANDLER(USB_IRQHandler)
HANDLER(ADC0_IRQHandler)
HANDLER(DAC0_IRQHandler)
HANDLER(I2C0_IRQHandler)
HANDLER(I2C1_IRQHandler)
HANDLER(GPIO_ODD_IRQHandler)
HANDLER(TIMER1_IRQHandler)
HANDLER(TIMER2_IRQHandler)
HANDLER(TIMER3_IRQHandler)
HANDLER(USART1_RX_IRQHandler)
HANDLER(USART1_TX_IRQHandler)
HANDLER(LESENSE_IRQHandler)
HANDLER(USART2_RX_IRQHandler)
HANDLER(USART2_TX_IRQHandler)
HANDLER(UART0_RX_IRQHandler)
HANDLER(UART0_TX_IRQHandler)
HANDLER(UART1_RX_IRQHandler)
HANDLER(UART1_TX_IRQHandler)
HANDLER(LEUART0_IRQHandler)
HANDLER(LEUART1_IRQHandler)
HANDLER(LETIMER0_IRQHandler)
HANDLER(PCNT0_IRQHandler)
HANDLER(PCNT1_IRQHandler)
HANDLER(PCNT2_IRQHandler)
HANDLER(RTC_IRQHandler)
HANDLER(CMU_IRQHandler)
HANDLER(VCMP_IRQHandler)
HANDLER(LCD_IRQHandler)
HANDLER(MSC_IRQHandler)
HANDLER(AES_IRQHandler)
HANDLER(EBI_IRQHandler)
HANDLER(EMU_IRQHandler)
