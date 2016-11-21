#ifndef LIBS_DRIVERS_UART_IUART_H_
#define LIBS_DRIVERS_UART_IUART_H_
#include "ecode.h"
#include <stdint.h>


class IUart {
public :
	virtual Ecode_t Read(uint8_t &buf)=0;
	virtual Ecode_t Write(uint8_t *buf, int size)=0;
	virtual Ecode_t Init()=0;

};

#endif /* LIBS_DRIVERS_UART_IUART_H_ */
