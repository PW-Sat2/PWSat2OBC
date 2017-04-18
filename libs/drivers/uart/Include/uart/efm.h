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

/**
 * @defgroup uart_efm UART interface for EFM MCU
 * @ingroup uart
 *
 * @{
 */

/**
 * @brief UART interface using EFM UART peripheral
 *
 * This class represent EFM peripheral
 */


class EFMUartInterface final : public IUartInterface{



public:
	EFMUartInterface();
	/**
	 * @brief Writes buffer to device
	 * @param[in] buffer Input buffer
	 */
	virtual UartResult Write(gsl::span<const uint8_t>data) override;
	/**
	 * @brief Reads from device
	 * @param[in] buffer Output buffer
	 */
	virtual UartResult Read(gsl::span<const uint8_t>data) override;
	/**
	 * Initializes UART interface and configures peripheral
	 */
	void Initialize(void);
	/**
	 * Deinitializes UART interface and configures peripheral
	 */
	void DeInitialize(void);
	/** @brief Output transfer finished flag */
	static constexpr OSEventBits TransferTXFinished = 1 << 1;
	/** @brief Input transfer finished flag */

	static constexpr OSEventBits TransferRXFinished = 1 << 0;
	/** @brief Input and output transfer finished flag */

	static constexpr OSEventBits TransferFinished = TransferRXFinished | TransferTXFinished;
	/** @brief Event group with transfer finished flags */
	OSEventGroupHandle _transferGroup;


private:
	/**
	 * Initializes DMA interface for UART periperial
	 */
	void InitializeDma(void);
	/**
	 * Initializes GPIO interface for UART periperial
	 */
	void InitializeGpio(void);


	/**
	 * @brief DMA callback called when transfer is finished. Always executes in ISR mode
	 * @param[in] channel Channel number
	 * @param[in] sequenceNo Sequence number
	 * @param[in] param User-specified param. In this case this pointer
	 * @return Ignored
	 */
	static bool TransmitDmaComplete(unsigned int channel,
			unsigned int sequenceNo,
			void *userParam);
};


}
}

#endif /* LIBS_DRIVERS_UART_UART_H_ */
