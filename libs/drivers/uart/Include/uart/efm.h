#ifndef LIBS_DRIVERS_UART_UART_H_
#define LIBS_DRIVERS_UART_UART_H_


#include <stdlib.h>
#include "dmadrv.h"
#include "em_usart.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "base/os.h"
#include "logger/logger.h"
#include <gsl/span>
#include "io_map.h"
#include "uart.h"


namespace drivers
{

namespace uart
    {

class EFMUartInterface final : public IUartInterface{



public:
	EFMUartInterface();
	virtual UartResult Write(gsl::span<const uint8_t>data) override;
	virtual UartResult Read(gsl::span<const uint8_t>data) override;
	void Initialize(void);
	void DeInitialize(void);
	static constexpr OSEventBits TransferTXFinished = 1 << 1;
	static constexpr OSEventBits TransferRXFinished = 1 << 0;
	static constexpr OSEventBits TransferFinished = TransferRXFinished | TransferTXFinished;
	OSEventGroupHandle _transferGroup;


private:
	void InitializeDma(void);
	void InitializeGpio(void);
	static bool TransmitDmaComplete(unsigned int channel,
			                        unsigned int sequenceNo,
			                        void *userParam);
	static bool ReceiveDmaComplete(unsigned int channel,
            unsigned int sequenceNo,
            void *userParam);
};


    }
}

#endif /* LIBS_DRIVERS_UART_UART_H_ */
