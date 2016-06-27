#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <stdio.h>

#include <em_device.h>
#include <em_usart.h>
#include <em_gpio.h>
#include <em_cmu.h>
#include <efm32gg_uart.h>

#include "uart.h"
#include "system_efm32gg.h"
#include "io_map.h"
#include "system.h"

static USART_TypeDef *uart = USART1;
QueueHandle_t usart1sink;

int8_t UARTSend(uint8_t * str, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++) {
        USART_Tx(uart, str[i]);
    }
    return size;
}

int8_t UARTInit(xQueueHandle sink)
{
    /* Initialize USART */
    USART_InitAsync_TypeDef init = {
        .enable         = usartDisable,     /* Disable RX/TX when init completed. */
        .refFreq        = 0,                /* Use current configured reference clock for configuring baudrate. */
        .baudrate       = USART1_BAUDRATE,  /* 9600 bits/s. */
        .oversampling   = usartOVS16,       /* 16x oversampling. */
        .databits       = usartDatabits8,   /* 8 databits. */
        .parity         = usartNoParity,    /* No parity. */
        .stopbits       = usartStopbits1,   /* 1 stopbit. */
        .mvdis          = false,            /* Do not disable majority vote. */
        .prsRxEnable    = false,            /* Not USART PRS input mode. */
        .prsRxCh        = usartPrsRxCh0     /* PRS channel 0. */
        
    };
    usart1sink = sink;
    CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
    CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);

    CMU_ClockEnable(cmuClock_HFPER, true);
    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(cmuClock_USART1, true);
    /* Use default location 0: TX - Pin F6, RX - Pin F7 */
    /* To avoid false start, configure output as high */
    GPIO_PinModeSet(USART1_PORT, USART1_TX, gpioModePushPull, 1);

    /* Define input, no filtering */
    GPIO_PinModeSet(USART1_PORT, USART1_RX, gpioModeInput, 0);

    /* Configure USART for basic async operation */
    USART_InitAsync(uart, &init);

    /* Enable pins at default location */
    uart->ROUTE = UART_LOCATION | UART_ROUTE_RXPEN | UART_ROUTE_TXPEN;

    /* Clear previous RX interrupts */
    USART_IntClear(uart, _USART_IF_MASK);
    NVIC_ClearPendingIRQ(USART1_RX_IRQn);
    USART_IntEnable(uart, USART_IF_RXDATAV);
    NVIC_EnableIRQ(USART1_RX_IRQn);

    /* Finally enable it */
    USART_Enable(uart, usartEnable);
    return 0;
}

void USART1_RX_IRQHandler(void)
{
    uint8_t data;
    BaseType_t woken = pdFALSE;
    while (uart->STATUS & USART_STATUS_RXDATAV) {
        data = (uint8_t) uart->RXDATA;
        xQueueSendToBackFromISR(usart1sink, &data, &woken);
    }

    portEND_SWITCHING_ISR(woken);
}
