#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <Uart.h>
#include "em_usart.h"
#include "em_gpio.h"
#include "dmadrv.h"



static SemaphoreHandle_t lineEndReceived = xSemaphoreCreateBinary();

Uart::Uart(Uart_Init &init):_init(init)
{

}

void Uart::InitializeDma(){
	this->uartClock   = cmuClock_USART1;
	this->txDmaSignal = dmadrvPeripheralSignal_USART1_TXBL;
	this->rxDmaSignal = dmadrvPeripheralSignal_USART1_RXDATAV;
	DMADRV_Init();
}

void Uart::InitializeGpio(){
	 txPort = (GPIO_Port_TypeDef)AF_USART1_TX_PORT(this->_init.portLocationTx);
	 rxPort = (GPIO_Port_TypeDef)AF_USART1_RX_PORT(this->_init.portLocationRx);
	 uint8_t txPin  = AF_USART1_TX_PIN(this->_init.portLocationTx);
	 uint8_t rxPin  = AF_USART1_RX_PIN(this->_init.portLocationRx);
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
	 CMU_ClockEnable(uartClock, true);
	 USART_InitAsync(_init.uart, &usartInit);
	 Uart::InitializeDma();
	 Uart::InitializeGpio();
	 _init.uart->ROUTE = USART_ROUTE_TXPEN
	                         | USART_ROUTE_RXPEN
	                         | (_init.portLocation
	                         << _USART_ROUTE_LOCATION_SHIFT);


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
void Uart::Read(uint8_t &data){


	 DMADRV_PeripheralMemory(this->rxDmaCh,
	        dmadrvPeripheralSignal_USART1_RXDATAV,
	        (void*)&data,
	        (void*)&USART1->RXDATA,
	        true,
	        sizeof(data),
	        dmadrvDataSize1,
	        NULL,
	        NULL);

	    NVIC_EnableIRQ(LEUART0_IRQn);

	    xSemaphoreTake(lineEndReceived, portMAX_DELAY);

	    NVIC_DisableIRQ(LEUART0_IRQn);

}

void Uart::Write(uint8_t &data){
	  DMADRV_MemoryPeripheral(this->txDmaCh,
	                          this->txDmaSignal,
							  (void *)&(this->_init.uart->TXDATA),
	                          (void*)&data,
	                          true,
	                          sizeof(data),
	                          dmadrvDataSize1,
	                          NULL,
							  NULL);

}

void USART1_IRQHandler(void)
{
    uint32_t flags = USART_IntGet(USART1);
    USART_IntClear(USART1, flags);

    xSemaphoreGiveFromISR(lineEndReceived, NULL);

}

