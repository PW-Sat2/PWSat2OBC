#ifndef LIBS_DRIVERS_UART_UART_H_
#define LIBS_DRIVERS_UART_UART_H_

#include "ecode.h"
#include <stdlib.h>
#include "dmadrv.h"
#include "em_usart.h"
#include "em_cmu.h"
#include "em_gpio.h"


#define ECODE_EMDRV_UARTDRV_OK                (ECODE_OK)
#define ECODE_EMDRV_UARTDRV_WAITING           (ECODE_EMDRV_UARTDRV_BASE | 0x00000001)
#define ECODE_EMDRV_UARTDRV_ILLEGAL_HANDLE    (ECODE_EMDRV_UARTDRV_BASE | 0x00000002)
#define ECODE_EMDRV_UARTDRV_PARAM_ERROR       (ECODE_EMDRV_UARTDRV_BASE | 0x00000003)
#define ECODE_EMDRV_UARTDRV_BUSY              (ECODE_EMDRV_UARTDRV_BASE | 0x00000004)
#define ECODE_EMDRV_UARTDRV_ILLEGAL_OPERATION (ECODE_EMDRV_UARTDRV_BASE | 0x00000005)
#define ECODE_EMDRV_UARTDRV_IDLE              (ECODE_EMDRV_UARTDRV_BASE | 0x00000008)
#define ECODE_EMDRV_UARTDRV_ABORTED           (ECODE_EMDRV_UARTDRV_BASE | 0x00000009)
#define ECODE_EMDRV_UARTDRV_QUEUE_FULL        (ECODE_EMDRV_UARTDRV_BASE | 0x0000000A)
#define ECODE_EMDRV_UARTDRV_QUEUE_EMPTY       (ECODE_EMDRV_UARTDRV_BASE | 0x0000000B)
#define ECODE_EMDRV_UARTDRV_PARITY_ERROR      (ECODE_EMDRV_UARTDRV_BASE | 0x0000000C)
#define ECODE_EMDRV_UARTDRV_FRAME_ERROR       (ECODE_EMDRV_UARTDRV_BASE | 0x0000000D)
#define ECODE_EMDRV_UARTDRV_DMA_ALLOC_ERROR   (ECODE_EMDRV_UARTDRV_BASE | 0x0000000E)
#define ECODE_EMDRV_UARTDRV_CLOCK_ERROR       (ECODE_EMDRV_UARTDRV_BASE | 0x0000000F)




//typedef void (*UART_Callback_t)(struct Uart_Init *Init,
//                                    Ecode_t transferStatus,
//                                    uint8_t *data,
//									uint32_t transferCount);



struct Uart_Init
{
USART_TypeDef             *uart;
uint32_t             baudRate;
uint8_t              portLocationTx;
uint8_t              portLocationRx;
uint8_t              portLocation;
USART_Stopbits_TypeDef    stopBits;
USART_Parity_TypeDef      parity;
USART_OVS_TypeDef         oversampling;
//UART_Callback_t callbackTx;
//UART_Callback_t callbackRx;
};


class Uart final{

Uart(Uart_Init &init);

public:
void Write(uint8_t &data);
void Read(uint8_t &data);
void Initialize(void);
void DeInitialize(void);

private:
	Uart_Init _init;
	uint8_t               rxDmaCh;
	uint8_t               txDmaCh;
	CMU_Clock_TypeDef          uartClock;
	DMADRV_PeripheralSignal_t  txDmaSignal;
	DMADRV_PeripheralSignal_t  rxDmaSignal;
	GPIO_Port_TypeDef txPort;
	GPIO_Port_TypeDef rxPort;
	void InitializeDma(void);
	void InitializeGpio(void);
};



#endif /* LIBS_DRIVERS_UART_UART_H_ */
