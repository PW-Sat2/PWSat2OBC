/***************************************************************************/ /**
  * @file
  * @brief   CubeComputer Test Program Includes.
  * @author  Pieter Botma
  * @version 1.12.06
  ******************************************************************************/

#ifndef __INCLUDES_H
#define __INCLUDES_H
#include <math.h>    //< Located in: CodeSourcery\Sourcery_CodeBench_Lite_for_ARM_EABI\arm-none-eabi\include
#include <stdbool.h> //< Located in: CodeSourcery\Sourcery_CodeBench_Lite_for_ARM_EABI\lib\gcc\arm-none-eabi\4.7.3\include
#include <stdint.h>  //< Located in: CodeSourcery\Sourcery_CodeBench_Lite_for_ARM_EABI\arm-none-eabi\include
#include <stdio.h>   //< Located in: CodeSourcery\Sourcery_CodeBench_Lite_for_ARM_EABI\arm-none-eabi\include
#include <stdlib.h>  //< Located in: CodeSourcery\Sourcery_CodeBench_Lite_for_ARM_EABI\arm-none-eabi\include
#include <string.h>  //< Located in: CodeSourcery\Sourcery_CodeBench_Lite_for_ARM_EABI\arm-none-eabi\include
#include <time.h>    //< Located in: CodeSourcery\Sourcery_CodeBench_Lite_for_ARM_EABI\arm-none-eabi\include

// SiliconLabs/EnergyMicro library
#include "em_chip.h"   //< Located in: CubeComputer-Workspace\libraries\emlib\inc
#include "em_device.h" //< Located in: CubeComputer-Workspace\libraries\Device\EnergyMicro\EFM32GG\Include

// component specific library
//#include "ff.h"				//< Located in:CubeComputer-Workspace\libraries\fatfs\inc
//#include "diskio.h"			//< Located in:CubeComputer-Workspace\libraries\fatfs\inc
//#include "microsdconfig.h"	//< Located in:CubeComputer-Workspace\libraries\fatfs\inc
//#include "microsd.h"		//< Located in:CubeComputer-Workspace\libraries\fatfs\inc

#include "lld.h" //< Located in:CubeComputer-Workspace\libraries\flashLib

//#include "MCP2515.h" //< Located in:CubeComputer-Workspace\libraries\canLib

// board support library
#include "bsp/bsp_acmp.h" //< Located in:CubeComputer-Workspace\libraries\bspLib\inc
#include "bsp/bsp_adc.h"  //< Located in:CubeComputer-Workspace\libraries\bspLib\inc
#include "bsp/bsp_boot.h" //< Located in:CubeComputer-Workspace\libraries\bspLib\inc
#include "bsp/bsp_can.h"  //< Located in:CubeComputer-Workspace\libraries\bspLib\inc
#include "bsp/bsp_dma.h"  //< Located in:CubeComputer-Workspace\libraries\bspLib\inc
#include "bsp/bsp_ebi.h"  //< Located in:CubeComputer-Workspace\libraries\bspLib\inc
#include "bsp/bsp_i2c.h"  //< Located in:CubeComputer-Workspace\libraries\bspLib\inc
#include "bsp/bsp_rtc.h"  //< Located in:CubeComputer-Workspace\libraries\bspLib\inc
#include "bsp/bsp_see.h"  //< Located in:CubeComputer-Workspace\libraries\bspLib\inc
#include "bsp/bsp_time.h" //< Located in:CubeComputer-Workspace\libraries\bspLib\inc
#include "bsp/bsp_uart.h" //< Located in:CubeComputer-Workspace\libraries\bspLib\inc
#include "bsp/bsp_unix.h" //< Located in:CubeComputer-Workspace\libraries\bspLib\inc
#include "bsp/bsp_wdg.h"  //< Located in:CubeComputer-Workspace\libraries\bspLib\inc

// application library
#include "background.h" //< Located in:CubeComputer-Workspace\Source
#include "comms.h"      //< Located in:CubeComputer-Workspace\Source
#include "globals.h"    //< Located in:CubeComputer-Workspace\Source
#include "tests.h"      //< Located in:CubeComputer-Workspace\Source
#endif                  // __INCLUDES_H
