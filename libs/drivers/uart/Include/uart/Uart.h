#ifndef LIBS_DRIVERS_UART_UART_H_
#define LIBS_DRIVERS_UART_UART_H_

#include "ecode.h"
#include <stdlib.h>
#include "dmadrv.h"
#include "em_usart.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "base/os.h"
#include "logger/logger.h"
#include <gsl/span>


namespace drivers
{

namespace uart
    {

//typedef void (*UART_Callback_t)(struct Uart_Init *Init,
//                                    Ecode_t transferStatus,
//                                    uint8_t *data,
//									uint32_t transferCount);
enum class UartResult{
	OK = 0,
	Timeout = -6,
	Failure = -8
};



struct IUartBus {
	virtual UartResult Write(gsl::span<const uint8_t>data)=0;
	virtual UartResult Read(gsl::span<const uint8_t>data)=0;
};





struct Uart_Init{
USART_TypeDef             *uart;
uint32_t             baudRate;
uint8_t              portLocationTx;
uint8_t              portLocationRx;
uint8_t              portLocation;
USART_Stopbits_TypeDef    stopBits;
USART_Parity_TypeDef      parity;
USART_OVS_TypeDef         oversampling;
USART_Databits_TypeDef dataBits;
//UART_Callback_t callbackTx;
//UART_Callback_t callbackRx;
};


class Uart final : public IUartBus{



public:
Uart(Uart_Init &init);
virtual UartResult Write(gsl::span<const uint8_t>data) override;
virtual UartResult Read(gsl::span<const uint8_t>data) override;
void Initialize(void);
void DeInitialize(void);

private:
	Uart_Init 				_init;
	CMU_Clock_TypeDef          uartClock;
	DMADRV_PeripheralSignal_t  txDmaSignal;
	DMADRV_PeripheralSignal_t  rxDmaSignal;
	GPIO_Port_TypeDef txPort;
	GPIO_Port_TypeDef rxPort;
	OSSemaphoreHandle txlock;
	OSSemaphoreHandle rxlock;
	void InitializeDma(void);
	void InitializeGpio(void);
};


    }
}

#endif /* LIBS_DRIVERS_UART_UART_H_ */
