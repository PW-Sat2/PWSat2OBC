#ifndef SRC_IO_MAP_H_
#define SRC_IO_MAP_H_

#define LED_PORT gpioPortE
#define LED0 2
#define LED1 3

// LEUART0
#define LEUART0_PORT gpioPortD
#define LEUART0_TX 4
#define LEUART0_RX 5
#define LEUART0_LOCATION LEUART_ROUTE_LOCATION_LOC0
#define LEUART0_BAUDRATE 9600


#define ADXRS453_SPI_USART_PORT USART1
#define ADXRS453_SPI_USART_ROUTE_LOCATION _USART_ROUTE_LOCATION_LOC1
#define GYRO0 	\
	{ 			\
	gpioPortD, /* cs port location*/			\
	3, /* cs pin location */			\
	}

#define GYRO1 	\
	{ 			\
	gpioPortD, /* cs port location*/			\
	6, /* cs pin location */			\
	}

#define GYRO2 	\
	{ 			\
	gpioPortC, /* cs port location*/			\
	6, /* cs pin location */			\
	}



#endif
