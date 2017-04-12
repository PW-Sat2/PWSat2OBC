#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <Uart.h>
#include "em_usart.h"
#include "em_gpio.h"
#include "dmadrv.h"
#include "system.h"
#include "base/os.h"
#include "event_groups.h"
#include "efm_support/api.h"
#include "efm_support/clock.h"
#include "efm_support/dma.h"

using namespace drivers::uart;

using namespace std::chrono_literals;
static void* RXPort = const_cast<uint32_t*>(&io_map::UART::Peripheral->RXDATA);
static void* TXPort = const_cast<uint32_t*>(&io_map::UART::Peripheral->TXDATA);
static unsigned int               	rxDmaCh;
static unsigned int              	txDmaCh;


bool EFMUartInterface::TransmitDmaComplete(unsigned int channel,
		                        unsigned int sequenceNo,
		                        void *userParam)
   {
	auto This = static_cast<Uart*>(userParam);
	(void)sequenceNo;
	if(channel == txDmaCh)
    System::EventGroupSetBitsISR(This->_transferGroup, TransferTXFinished);
	else {
		System::EventGroupSetBitsISR(This->_transferGroup, TransferRXFinished);
	}
    System::EndSwitchingISR();
	return true;
  }




EFMUartInterface::EFMUartInterface(){

}


void EFMUartInterface::InitializeDma(){
	efm::dma::Init();
	efm::dma::AllocateChannel(&rxDmaCh, nullptr);
	efm::dma::AllocateChannel(&txDmaCh, nullptr);
}

void EFMUartInterface::Initialize(){
	 USART_InitAsync_TypeDef usartInit = USART_INITASYNC_DEFAULT;
	 usartInit.baudrate = 9600;
	 usartInit.stopbits = usartStopbits1;
	 usartInit.parity = usartNoParity;
	 usartInit.oversampling = usartOVS16;
	 usartInit.databits = (USART_Databits_TypeDef)USART_FRAME_DATABITS_EIGHT;
	 efm::cmu::ClockEnable(efm::Clock(io_map::UART::Peripheral), true);
	 efm::usart::InitAsync(io_map::UART::Peripheral, &usartInit);
	 efm::usart::IntClear(io_map::UART::Peripheral, ~0x0);
	 Uart::InitializeDma();
	 efm::usart::AmendRoute(io_map::UART::Peripheral,
	         USART_ROUTE_CLKPEN | USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | (io_map::UART::Location << _USART_ROUTE_LOCATION_SHIFT));
	 efm::usart::Enable(io_map::UART::Peripheral, usartEnable);
	 this->_transferGroup = System::CreateEventGroup();
}
void EFMUartInterface::DeInitialize(){
	 efm::dma::StopTransfer(rxDmaCh);
	 efm::dma::StopTransfer(rxDmaCh);
	 efm::dma::FreeChannel(rxDmaCh);
	 efm::dma::FreeChannel(rxDmaCh);
	 efm::cmu::ClockEnable(efm::Clock(io_map::UART::Peripheral), false);
	 efm::usart::Command(io_map::UART::Peripheral, USART_CMD_RXDIS | USART_CMD_TXDIS);
}

UartResult EFMUartInterface::Read(gsl::span<const uint8_t>data){
    System::EventGroupClearBits(this->_transferGroup, TransferRXFinished);

    efm::usart::Command(io_map::UART::Peripheral, USART_CMD_CLEARRX | USART_CMD_CLEARTX);
    efm::usart::IntClear(io_map::UART::Peripheral, efm::usart::IntGet(io_map::UART::Peripheral));
    efm::dma::PeripheralMemory(rxDmaCh,
            efm::DMASignal<efm::DMASignalUSART::RXDATAV>(io_map::UART::Peripheral),
			(void*)data.data(),
            RXPort,
            true,
			data.length(),
            dmadrvDataSize1,
			TransmitDmaComplete,
            this);
	 System::EventGroupWaitForBits(this->_transferGroup, TransferFinished, true, true, 1s);
	 return UartResult::OK;
}

UartResult EFMUartInterface::Write(gsl::span<const uint8_t>data){
    System::EventGroupClearBits(this->_transferGroup, TransferTXFinished);

    efm::usart::Command(io_map::UART::Peripheral, USART_CMD_CLEARRX | USART_CMD_CLEARTX);
    efm::usart::IntClear(io_map::UART::Peripheral, efm::usart::IntGet(io_map::UART::Peripheral));
	efm::dma::MemoryPeripheral(txDmaCh,
	        efm::DMASignal<efm::DMASignalUSART::TXBL>(io_map::SPI::Peripheral),
	        TXPort,
	        const_cast<uint8_t*>(data.data()),
	        true,
			 data.length(),
	        dmadrvDataSize1,
			TransmitDmaComplete,
	        this);
	 System::EventGroupWaitForBits(this->_transferGroup, TransferFinished, true, true, 2s);
	 return UartResult::OK;
}
