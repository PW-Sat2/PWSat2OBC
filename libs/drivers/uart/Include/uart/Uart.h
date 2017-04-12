#ifndef LIBS_DRIVERS_UART_INCLUDE_UART_UART_H_
#define LIBS_DRIVERS_UART_INCLUDE_UART_UART_H_

#include "system.h"
#include <gsl/span>
#include "em_usart.h"
enum class UartResult{
	OK = 0,
	Timeout = -6,
	Failure = -8
};


namespace drivers
{

namespace uart
    {

struct IUartInterface {
	virtual UartResult Write(gsl::span<const uint8_t>data)=0;
	virtual UartResult Read(gsl::span<const uint8_t>data)=0;
};


    }
}
#endif /* LIBS_DRIVERS_UART_INCLUDE_UART_UART_H_ */
