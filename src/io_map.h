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

#define ADXRS453_SPI                                              \
{                                                                         \
  USART1,                       /* USART port                       */    \
  _USART_ROUTE_LOCATION_LOC1,   /* USART pins location number ||  MOSI-PD0, MISO-PD1, CLK-PD2, CS-PD3       */    \
  1000000,                      /* Bitrate                          */    \
  8,                            /* Frame length                     */    \
  0,                            /* Dummy tx value for rx only funcs */    \
  spidrvMaster,                 /* SPI mode                         */    \
  spidrvBitOrderMsbFirst,       /* Bit order on bus                 */    \
  spidrvClockMode0,             /* SPI clock/phase mode             */    \
  spidrvCsControlAuto,          /* CS controlled by the driver      */    \
  spidrvSlaveStartDelayed          \
}



#endif
