/**************************************************************************/ /**
  * @file
  * @brief XMODEM protocol
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
#include "xmodem.h"
#include <stdio.h>
#include "em_system.h"
#include "bsp/bsp_boot.h"
#include "bsp/bsp_ebi.h"
#include "bsp/bsp_uart.h"
#include "crc.h"
#include "flash.h"
#include "lld.h"

#define ALIGNMENT(base, align) (((base) + ((align)-1)) & (~((align)-1)))

/* Packet storage. Double buffered version. */
uint8_t rawPacket[5][ALIGNMENT(sizeof(XMODEM_packet), 4)] __attribute__((aligned(4)));

/**************************************************************************/ /**
  * @brief Verifies checksum, packet numbering and
  * @param pkt The packet to verify
  * @param sequenceNumber The current sequence number.
  * @returns -1 on packet error, 0 otherwise
  *****************************************************************************/
int XMODEM_verifyPacketChecksum(XMODEM_packet* pkt, int sequenceNumber)
{
    uint16_t packetCRC;
    uint16_t calculatedCRC;

    /* Check the packet number integrity */
    if (pkt->packetNumber + pkt->packetNumberC != 255)
    {
        return -1;
    }

    /* Check that the packet number matches the excpected number */
    if (pkt->packetNumber != (sequenceNumber % 256))
    {
        return -1;
    }

    calculatedCRC = CRC_calc((uint8_t*)pkt->data, (uint8_t*)&(pkt->crcHigh));
    packetCRC = pkt->crcHigh << 8 | pkt->crcLow;

    /* Check the CRC value */
    if (calculatedCRC != packetCRC)
    {
        return -1;
    }
    return 0;
}

/**************************************************************************/ /**
  * @brief Starts a XMODEM upload.
  *
  * @param index
  *   The index where data should be saved in the boot table.
  *
  *****************************************************************************/
uint32_t XMODEM_upload(uint8_t index)
{
    XMODEM_packet* pkt;
    uint32_t offset, i;
    uint32_t byte;
    uint32_t sequenceNumber = 1;
    uint8_t *base, data;

    ((void)base);
    ((void)data);

    // Erase sectors in boot table for specified index
    if (index != 0)
    {
        for (offset = 0; offset < BOOT_TABLE_ENTRY_SIZE; offset += BSP_EBI_FLASH_LSECTOR_SIZE)
            lld_SectorEraseOp((uint8_t*)(BOOT_TABLE_BASE), (BOOT_getOffsetEntry(index) + offset));
    }

    // Send one start transmission packet. Wait for a response. If there is no
    // response, we resend the start transmission packet.
    // Note: This is a fairly long delay between retransmissions(~6 s).
    while (1)
    {
        BSP_UART_txByte(BSP_UART_DEBUG, XMODEM_NCG);
        for (i = 0; i < 10000000; i++)
        {
            if (BSP_UART_DEBUG->STATUS & USART_STATUS_RXDATAV)
            {
                goto xmodem_transfer;
            }
        }
    }
xmodem_transfer:
    while (1)
    {
        // Swap buffer for packet buffer
        pkt = (XMODEM_packet*)rawPacket[sequenceNumber & 1];

        // Fetch the first byte of the packet explicitly, as it defines the
        // rest of the packet
        pkt->header = USART_Rx(BSP_UART_DEBUG);

        // Check for end of transfer
        if (pkt->header == XMODEM_EOT)
        {
            // Acknowledget End of transfer
            BSP_UART_txByte(BSP_UART_DEBUG, XMODEM_ACK);
            break;
        }

        // If the header is not a start of header (SOH), then cancel
        // the transfer.
        if (pkt->header != XMODEM_SOH)
        {
            return 0;
        }

        // Fill the remaining bytes packet
        // Byte 0 is padding, byte 1 is header
        for (byte = 2; byte < sizeof(XMODEM_packet); byte++)
        {
            if (byte == 60)
            {
                *(((uint8_t*)pkt) + byte) = USART_Rx(BSP_UART_DEBUG);
            }
            else if (byte == 45)
            {
                *(((uint8_t*)pkt) + byte) = USART_Rx(BSP_UART_DEBUG);
            }
            else if (byte == 30)
            {
                *(((uint8_t*)pkt) + byte) = USART_Rx(BSP_UART_DEBUG);
            }
            else if (byte == 15)
            {
                *(((uint8_t*)pkt) + byte) = USART_Rx(BSP_UART_DEBUG);
            }
            else
            {
                *(((uint8_t*)pkt) + byte) = USART_Rx(BSP_UART_DEBUG);
            }
        }

        if (XMODEM_verifyPacketChecksum(pkt, sequenceNumber) != 0)
        {
            /* On a malformed packet, we send a NAK, and start over */
            BSP_UART_txByte(BSP_UART_DEBUG, XMODEM_NAK);
            continue;
        }

        if (index == 0)
        {
            i = 0;
            volatile uint8_t* area = (uint8_t*)(BOOT_SAFEMODE_BASE_DATA + ((sequenceNumber - 1) * XMODEM_DATA_SIZE));

            do
            {
                // Unlock commands
                *((volatile uint8_t*)(BOOT_SAFEMODE_BASE_DATA + 0x5555)) = 0xAA;
                *((volatile uint8_t*)(BOOT_SAFEMODE_BASE_DATA + 0x2AAA)) = 0x55;
                *((volatile uint8_t*)(BOOT_SAFEMODE_BASE_DATA + 0x5555)) = 0xA0;

                // Write to page (can only write in 64 bytes at a time)
                do
                {
                    *(area + i) = pkt->data[i];
                    i++;
                } while ((i < XMODEM_DATA_SIZE) && ((i % 64) != 0));

                // Poll write sequence completion
                while (((*(area + i - 1)) & 0x80) != (pkt->data[i - 1] & 0x80))
                    ;
            } while (i < XMODEM_DATA_SIZE);
        }
        // Write data to external FLASH, i.e. Nominal Mode
        else
        {
            base = (uint8_t*)BOOT_TABLE_BASE;
            offset = (BOOT_getOffsetProgram(index) + (sequenceNumber - 1) * XMODEM_DATA_SIZE);

            // Write to flash (write 1 byte at a time)
            for (i = 0; i < XMODEM_DATA_SIZE; i++)
            {
                data = pkt->data[i];
                lld_ProgramOp(base, (offset + i), data);
            }
        }

        sequenceNumber++;

        // Send ACK
        BSP_UART_txByte(BSP_UART_DEBUG, XMODEM_ACK);
    }

    return ((sequenceNumber - 1) * XMODEM_DATA_SIZE);
}
