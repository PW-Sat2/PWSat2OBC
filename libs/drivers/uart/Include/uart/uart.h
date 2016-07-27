#ifndef _UART_H_
#define _UART_H_

void UARTInit(xQueueHandle sink);

void UARTSend(uint8_t * str, uint32_t size);

#endif /* _UART_H_ */
