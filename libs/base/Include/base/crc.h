/**************************************************************************/ /**
  * @file
  * @brief CRC16 routines for XMODEM and verification.
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

#ifndef _CRC_H
#define _CRC_H

#include <stdint.h>
#include <gsl/span>

/**
 * @brief Calculates CRC for given area
 * @param start Pointer to first byte of area
 * @param end Pointer to first byte after area
 * @return Calculated crc
 */
uint16_t CRC_calc(uint8_t* start, uint8_t* end);

/**
 * @brief Calculates CRC for given area
 * @param buffer Span containing area
 * @return Calculated crc
 */
uint16_t CRC_calc(gsl::span<const uint8_t> buffer);

#endif
