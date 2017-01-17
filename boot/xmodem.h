/**************************************************************************//**
 * @file
 * @brief XMODEM prototypes and definitions
 * @author Energy Micro AS
* @version 1.63
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2009 Energy Micro AS, http://www.energymicro.com</b>
 ******************************************************************************
 *
 * This source code is the property of Energy Micro AS. The source and compiled
 * code may only be used on Energy Micro "EFM32" microcontrollers.
 *
 * This copyright notice may not be removed from the source code nor changed.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
 * obligation to support this Software. Energy Micro AS is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Energy Micro AS will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 *****************************************************************************/

#ifndef _XMODEM_H
#define _XMODEM_H

#include <stdint.h>

#define XMODEM_SOH                1
#define XMODEM_EOT                4
#define XMODEM_ACK                6
#define XMODEM_NAK                21
#define XMODEM_CAN                24
#define XMODEM_NCG                67

#define XMODEM_DATA_SIZE          128

//#define XMODEM_USER_PAGE_START    0x0FE00000
//#define XMODEM_USER_PAGE_END      0x0FE00200

//#define XMODEM_LOCK_PAGE_START    0x0FE04000
//#define XMODEM_LOCK_PAGE_END      0x0FE04200

/* The maximum flash size of any EFM32 part */
//#define MAX_SIZE_OF_FLASH         (1024*1024)     /* 1 MB */

typedef struct
{
  uint8_t padding; /* Padding to make sure data is 32 bit aligned. */
  uint8_t header;
  uint8_t packetNumber;
  uint8_t packetNumberC;
  uint8_t data[XMODEM_DATA_SIZE];
  uint8_t crcHigh;
  uint8_t crcLow;
} XMODEM_packet;

uint32_t XMODEM_upload(uint8_t index);

#endif
