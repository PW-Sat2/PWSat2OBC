#include <stdint.h>
#include <em_device.h>
#include <core_cm3.h>
#include "swo/swo.h"
#include "system.h"

extern "C" void prvGetRegistersFromStack(uint32_t* pulFaultStackAddress)
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

    SwoPrintfOnChannel(
        3, "CFSR: 0x%X\nHFSR: 0x%X\nMMFAR: 0x%X\nBFAR: 0x%X\nLR: 0x%X\nPC: 0x%X\nPSR: 0x%X", cfsr, hfsr, mmfar, bfar, lr, pc, psr);

    for (;;)
        ;
}

/* The fault handler implementation calls a function called
prvGetRegistersFromStack(). */
__attribute__((naked)) void HardFault_Handler(void)
{
    __asm volatile(" tst lr, #4                                                \n"
                   " ite eq                                                    \n"
                   " mrseq r0, msp                                             \n"
                   " mrsne r0, psp                                             \n"
                   " ldr r1, [r0, #24]                                         \n"
                   " ldr r2, handler2_address_constHF                          \n"
                   " bx r2                                                     \n"
                   " handler2_address_constHF: .word prvGetRegistersFromStack  \n");
}
