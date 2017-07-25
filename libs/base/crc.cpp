/**************************************************************************/ /**
  * @file
  * @brief CRC calculation routines
  * @author Energy Micro AS
  * @version 1.63
  */
#include "crc.h"

/**************************************************************************/ /**
  * @brief
  *   This function calculates the CRC-16-CCIT checksum of a memory range.
  *
  * @note
  *   This implementation uses an initial value of 0, while some implementations
  *   of CRC-16-CCIT uses an initial value of 0xFFFF. If you wish to
  *   precalculate the CRC before uploading the binary to the bootloader you
  *   can use this function. However, keep in mind that the 'v' and 'c' commands
  *   computes the crc of the entire flash, so any bytes not used by your
  *   application will have the value 0xFF.
  *
  * @param start
  *   Pointer to the start of the memory block
  *
  * @param end
  *   Pointer to the end of the block. This byte is not included in the computed
  *   CRC.
  *
  * @return
  *   The computed CRC value.
  *****************************************************************************/
uint16_t CRC_calc(uint8_t* start, uint8_t* end)
{
    uint16_t crc = 0;
    uint8_t* data;

    for (data = start; data < end; data++)
    {
        crc = (crc >> 8) | (crc << 8);
        crc ^= *data;
        crc ^= (crc & 0xff) >> 4;
        crc ^= crc << 12;
        crc ^= (crc & 0xff) << 5;
    }
    return crc;
}

uint16_t CRC_calc(gsl::span<const uint8_t> buffer)
{
    uint16_t crc = 0;

    for (auto data : buffer)
    {
        crc = (crc >> 8) | (crc << 8);
        crc ^= data;
        crc ^= (crc & 0xff) >> 4;
        crc ^= crc << 12;
        crc ^= (crc & 0xff) << 5;
    }
    return crc;
}
