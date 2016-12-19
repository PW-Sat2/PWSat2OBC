#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <Uart.h>
#include "em_usart.h"
#include "em_gpio.h"
#include "dmadrv.h"


using namespace drivers::uart;


static unsigned int               	rxDmaCh;
static unsigned int              	txDmaCh;

static bool TransmitDmaComplete(unsigned int channel,
		                        unsigned int sequenceNo,
		                        void *userParam)
   {
	(void)channel;
	(void)sequenceNo;
	(void)userParam;
	return true;
  }

static bool ReceiveDmaComplete(unsigned int channel,
		                        unsigned int sequenceNo,
		                        void *userParam)
   {
	(void)channel;
	(void)sequenceNo;
	(void)userParam;
	return true;
  }


Uart::Uart(Uart_Init &init):_init(init){}

void Uart::InitializeDma(){
	this->uartClock   = cmuClock_UART1;
	this->txDmaSignal = dmadrvPeripheralSignal_USART1_TXBL;
	this->rxDmaSignal = dmadrvPeripheralSignal_USART1_RXDATAV;
	DMADRV_Init();
	DMADRV_AllocateChannel(&rxDmaCh, NULL);
	DMADRV_AllocateChannel(&txDmaCh, NULL);

}

void Uart::InitializeGpio(){
	 txPort = (GPIO_Port_TypeDef)AF_USART1_TX_PORT(this->_init.portLocation);
	 rxPort = (GPIO_Port_TypeDef)AF_USART1_RX_PORT(this->_init.portLocation);
	 uint8_t txPin  = AF_USART1_TX_PIN(this->_init.portLocation);
	 uint8_t rxPin  = AF_USART1_RX_PIN(this->_init.portLocation);
	 GPIO_PinModeSet(this->txPort, txPin, gpioModePushPull, 1);
	 GPIO_PinModeSet(this->rxPort, rxPin, gpioModeInputPull, 1);
}


void Uart:: Initialize(){
	 USART_InitAsync_TypeDef usartInit = USART_INITASYNC_DEFAULT;
	 usartInit.baudrate = _init.baudRate;
	 usartInit.stopbits = _init.stopBits;
	 usartInit.parity = _init.parity;
	 usartInit.oversampling = _init.oversampling;
	 usartInit.databits = (USART_Databits_TypeDef)USART_FRAME_DATABITS_EIGHT;
	 CMU_ClockEnable(cmuClock_HFPER, true);
	 CMU_ClockEnable(cmuClock_GPIO, true);
	 CMU_ClockEnable(cmuClock_USART1, true);
	 USART_InitAsync(_init.uart, &usartInit);
	 USART_IntClear(_init.uart, ~0x0);
	 Uart::InitializeDma();
	 Uart::InitializeGpio();
	 _init.uart->ROUTE = USART_ROUTE_TXPEN
	                         | USART_ROUTE_RXPEN
	                         | (_init.portLocation
	                         << _USART_ROUTE_LOCATION_SHIFT);

	 USART_Enable(_init.uart, usartEnableTx);

}
void Uart::DeInitialize(){
	 DMADRV_StopTransfer(rxDmaCh);
	 DMADRV_StopTransfer(txDmaCh);
	 DMADRV_FreeChannel(txDmaCh);
	 DMADRV_FreeChannel(rxDmaCh);
	 DMADRV_DeInit();
	 CMU_ClockEnable(uartClock, false);
	 this->_init.uart->CMD = USART_CMD_RXDIS;
	 this->_init.uart->CMD = USART_CMD_TXDIS;
}

UartResult Uart::Read(gsl::span<const uint8_t>data){
	uint8_t *InData= const_cast<uint8_t*>(data.data());
	Lock lock(this->rxlock, MAX_DELAY);

		if (!lock())
		    {
		        //LOGF(LOG_LEVEL_ERROR, "[UART] Taking receive semaphore failed.");
		        return UartResult::Failure;
		    }

	 DMADRV_PeripheralMemory(rxDmaCh,
	        dmadrvPeripheralSignal_USART1_RXDATAV,
	        (void*)InData,
	        (void*)&USART1->RXDATA,
	        true,
	        sizeof(data),
	        dmadrvDataSize1,
			ReceiveDmaComplete,
	        NULL);
	 return UartResult::OK;
}

UartResult Uart::Write(gsl::span<const uint8_t>data){
	uint8_t *InData= const_cast<uint8_t*>(data.data());
	Lock lock(this->txlock, MAX_DELAY);

	if (!lock())
	    {
	        //LOGF(LOG_LEVEL_ERROR, "[UART] Taking transmit semaphore failed.");
	        return UartResult::Failure;
	    }

	DMADRV_MemoryPeripheral(txDmaCh,
	                          this->txDmaSignal,
							  (void *)&(this->_init.uart->TXDATA),
	                          (void*)InData,
	                          true,
	                          data.length(),
	                          dmadrvDataSize1,
							  TransmitDmaComplete,
							  NULL);
	 return UartResult::OK;
}
