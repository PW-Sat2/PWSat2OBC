/***************************************************************************/ /**
  * @file
  * @brief   CubeComputer Test Program Includes.
  * @author  Pieter Botma
  * @version 1.12.06
  ******************************************************************************/

#ifndef __INCLUDES_H
#define __INCLUDES_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// energy micro library
#include "em_system.h"
#include "em_adc.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_dma.h"
#include "em_ebi.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_i2c.h"
#include "em_msc.h"
#include "em_rtc.h"
#include "em_usart.h"
#include "em_wdog.h"

// component specific library
//#include "ff.h" 				//PW-SAT: Not using FatFS
#include "bsp/MCP2515.h"
//#include "diskio.h" 			//PW-SAT: Not using FatFS
#include "lld.h"
//#include "microsd.h"			//PW-SAT: Not using MicroSD
//#include "microsdconfig.h"	//PW-SAT: Not using MicroSD

// board support library
#include "bsp/bsp_acmp.h"
#include "bsp/bsp_boot.h"
#include "bsp/bsp_dma.h"
#include "bsp/bsp_ebi.h"
#include "bsp/bsp_rtc.h"
#include "bsp/bsp_uart.h"
#include "bsp/bsp_wdg.h"

// application library
#include "background.h"
#include "base/crc.h"
#include "comms.h"
#include "flash.h"
#include "xmodem.h"

#define FIRMWARE_MAJOR 2
#define FIRMWARE_MINOR 1

#endif // __INCLUDES_H
