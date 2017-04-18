#ifndef LIBS_DRIVERS_UART_INCLUDE_UART_UART_H_
#define LIBS_DRIVERS_UART_INCLUDE_UART_UART_H_

#include "system.h"
#include <gsl/span>
#include "em_usart.h"
namespace drivers
{

namespace uart
{
/**
 * @defgroup uart UART driver wrapper
 * @ingroup perhipheral_drivers
 *
 * This module defines C++ friendly UART interface
 *
 * @{
 */



/**
 * @brief Possible results of transfer
 */
enum class UartResult{
	/** @brief Transfer completed successfully. */
	OK = 0,
			/** @brief Device connected to UART do not response */
	Timeout = -6,
			/** @brief General UART error */
	Failure = -8
};

/**
 * @brief UART interface
 */



struct IUartInterface {
	/**
	 * @brief Writes buffer to device
	 * @param[in] buffer Input buffer
	 */
	virtual UartResult Write(gsl::span<const uint8_t>data)=0;
	/**
	 * @brief Reads from device
	 * @param[in] buffer Output buffer
	 */
	virtual UartResult Read(gsl::span<const uint8_t>data)=0;
};





}
}
#endif /* LIBS_DRIVERS_UART_INCLUDE_UART_UART_H_ */
