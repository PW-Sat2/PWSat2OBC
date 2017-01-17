#ifndef LLD_TARGET_SPECIFIC_INC_H
#define LLD_TARGET_SPECIFIC_INC_H

/* lld_target_specific.h - Source Code for Spansion's Low Level Driver */

/**************************************************************************
* Copyright (C)2011 Spansion LLC. All Rights Reserved.
*
* This software is owned and published by:
* Spansion LLC, 915 DeGuigne Dr. Sunnyvale, CA  94088-3453 ("Spansion").
*
* BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND
* BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
*
* This software constitutes driver source code for use in programming Spansion's
* Flash memory components. This software is licensed by Spansion to be adapted only
* for use in systems utilizing Spansion's Flash memories. Spansion is not be
* responsible for misuse or illegal use of this software for devices not
* supported herein.  Spansion is providing this source code "AS IS" and will
* not be responsible for issues arising from incorrect user implementation
* of the source code herein.
*
* SPANSION MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
* REGARDING THE SOFTWARE, ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED
* USE, INCLUDING, WITHOUT LIMITATION, NO IMPLIED WARRANTY OF MERCHANTABILITY,
* FITNESS FOR A  PARTICULAR PURPOSE OR USE, OR NONINFRINGEMENT.  SPANSION WILL
* HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT, NEGLIGENCE OR
* OTHERWISE) FOR ANY DAMAGES ARISING FROM USE OR INABILITY TO USE THE SOFTWARE,
* INCLUDING, WITHOUT LIMITATION, ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA, SAVINGS OR PROFITS,
* EVEN IF SPANSION HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
*
* This software may be replicated in part or whole for the licensed use,
* with the restriction that this Copyright notice must be included with
* this software, whether used in part or whole, at all times.
********************************************************************************/
//#include "S29GLxxxS.h"
//#include "S29GLxxxR.h"
//#include "S29GLxxxP.h"
//#include "S29GLxxxN.h"
//#include "S29WSxxxN.h"
//#include "S29WSxxxP.h"
//#include "S29WSxxxR.h"
//#include "S29VSxxxR.h"
//#include "S29VS064R.h"
//#include "S29NSxxxR.h"
//#include "S29XSxxxS.h"
//#include "S29ALxxxD.h"
//#include "S29ALxxxJ.h"
#include "device/S29JLxxxJ.h"
//#include "S29CLxxxJ.h"

/**********************************************************
* Enable trace buffer macro.                              *
**********************************************************/
//#define TRACE
//#define EXTEND_ADDR

/**********************************************************
* Define LLD target device: flash.                        *
***********************************************************/
#define LLD_DEV_FLASH
//#define LLD_DEV_SIM

/****************************************************
*		Define how to access your hardware.         *
* Describe to the LLD the flash chip configuration  *
* un-comment the one that matches your system       *
****************************************************/
// #define LLD_CONFIGURATION_X16_AS_X16		/* no-interleaving, a single x16 device in x16 mode  */
// #define LLD_CONFIGURATION_X8X16_AS_X16   /* no-interleaving, a single x8/x16 device in x16 mode */
#define LLD_CONFIGURATION_X8X16_AS_X8 /* no-interleaving, a single x8/x16 device in x8 mode */
// #define LLD_CONFIGURATION_X16_AS_X32     /* two x16 devices interleaved to form x32 */
// #define LLD_CONFIGURATION_X8X16_AS_X32   /* two x8/x16 devices interleaved to form x32 */
// #define LLD_CONFIGURATION_X8_AS_X8       /* no-interleaving, a single x8 device in x8 mode
// #define LLD_CONFIGURATION_X8_AS_X32      /* special case when four X8X16 devices in X8 mode interleaving to form X32 */
// #define LLD_CONFIGURATION_X8_AS_X16      /* special case when two X8X16 devices in X8 mode interleaving to form X16 */
// #define LLD_CONFIGURATION_X32_AS_X32		/* no-interleaving, a single x32 device in x32 mode  */

/************************************************************************
* determines whether or not your system supports timestamp in the trace *
* can also be used to turn off printing of the timestamp in the trace   *
************************************************************************/
/* #define PRINT_TIMESTAMP	*/

/************************************************************************
* Creates lld_printf.log file and writes all the LLD_PRINTFs to it      *
************************************************************************/
/* #define LLD_PRINTF_LOGFILE */

/************************************************************************
* Pause between Erase Suspend cmd                                       *
************************************************************************/
/* #define PAUSE_BETWEEN_ERASE_SUSPENDS*/

/*************************************************************************
* Displays file name and line number information when an LLD_ASSERT call *
* fails.  This information is displayed in LLDCmdlineAssert().           *
*************************************************************************/
/* #define ASSERT_DIAGNOSTICS */

/*************************************************************************
* enables code to execute commands from a file instead of directly from *
* the command line.
**************************************************************************/
/* #define ENABLE_SCRIPTING_MACRO */

/***************************************************
* Enable NST (NOR Supper Tests)
****************************************************/
/* #define NST_TESTS */

/***************************************************
* Enable CATest (Competitive Analysis Tests)
****************************************************/
/* #define CA_TEST */

/********************************************************
* Enable if user wants to implements its own FLASH_RD/WR
*********************************************************/
/* #define USER_SPECIFIC_CMD */

#endif /* LLD_TARGET_SPECIFIC_INC_H */
