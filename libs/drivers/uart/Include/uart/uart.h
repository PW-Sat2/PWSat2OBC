#ifndef _UART_H_
#define _UART_H_

int8_t UARTInit(xQueueHandle sink);

int8_t UARTSend(uint8_t * str, uint32_t size);

#endif /* _UART_H_ */
