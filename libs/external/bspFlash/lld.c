/* lld.c - Source Code for Spansion's Low Level Driver */
/**************************************************************************
* Copyright (C)2011 Spansion LLC. All Rights Reserved .
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
*/
#include "lld.h"


#ifndef LLD_STATUS_REG
FLSTATE FLSTATEOP = FLSTATE_NOT_BUSY;
unsigned char g_suspend_state = FLRESUME;
ADDRESS g_suspend_sector, g_writebuffer_sector;

#define DQ1_MASK   (0x02 * LLD_DEV_MULTIPLIER)  /* DQ1 mask for all interleave devices */
#define DQ2_MASK   (0x04 * LLD_DEV_MULTIPLIER)  /* DQ2 mask for all interleave devices */
#define DQ3_MASK   (0x08 * LLD_DEV_MULTIPLIER)  /* DQ3 mask for all interleave devices */
#define DQ5_MASK   (0x20 * LLD_DEV_MULTIPLIER)  /* DQ5 mask for all interleave devices */
#define DQ6_MASK   (0x40 * LLD_DEV_MULTIPLIER)  /* DQ6 mask for all interleave devices */
#define DQ6_TGL_DQ1_MASK (dq6_toggles >> 5) 	/* Mask for DQ1 when device DQ6 toggling */
#define DQ6_TGL_DQ5_MASK (dq6_toggles >> 1) 	/* Mask for DQ5 when device DQ6 toggling */
#endif

/* private functions */

/* Public Functions  */
/******************************************************************************
*
* lld_GetVersion - Get LLD Version Number in string format.
*
*  PARAMETERS:  LLD_CHAR[] versionStr
*               Pointer to an empty char array. The size of the array has to be at
*               least 9 in order to avoid buffer overflow.
*  RETURNS: version number in string returned in versionStr
*
* ERRNO:
*/
void lld_GetVersion( LLD_CHAR versionStr[])
{
   LLD_CHAR*  pVer = (LLD_CHAR*)LLD_VERSION;

   if (versionStr)
   {
       while (*pVer) *versionStr++ = *pVer++;
       *versionStr = 0;
   }
}

/******************************************************************************
*
* lld_InitCmd - Initialize LLD
*
*
* RETURNS: void
*
* ERRNO:
*/
void lld_InitCmd
(
FLASHDATA * base_addr			/* device base address in system */
)
{
#if (defined S29GLxxxP)
	FLSTATEOP = FLSTATE_NOT_BUSY;
#endif
}

/******************************************************************************
*
* lld_ResetCmd - Writes a Software Reset command to the flash device
*
*
* RETURNS: void
*
* ERRNO:
*/

void lld_ResetCmd
(
FLASHDATA * base_addr   /* device base address in system */
)
{
#ifdef LLD_RESET_CMD
  /* Write Software RESET command */
  FLASH_WR(base_addr, 0, NOR_RESET_CMD);
  lld_InitCmd(base_addr);
#endif
}

#ifdef LLD_SECTOR_ERASE_CMD_1
/******************************************************************************
*
* lld_SectorEraseCmd - Writes a Sector Erase Command to Flash Device
*
* This function only issues the Sector Erase Command sequence.
* Erase status polling is not implemented in this function.
*
* RETURNS: void
*
*/

void lld_SectorEraseCmd
(
FLASHDATA * base_addr,                   /* device base address in system */
ADDRESS offset                           /* address offset from base address */
)
{

  /* Issue unlock sequence command */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_ERASE_SETUP_CMD);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  /* Write Sector Erase Command to Offset */
  FLASH_WR(base_addr, offset, NOR_SECTOR_ERASE_CMD);

#if (defined S29GLxxxP)
  FLSTATEOP = FLSTATE_ERASE;
#endif
}
#endif // LLD_SECTOR_ERASE_CMD_1
#ifdef LLD_SECTOR_ERASE_CMD_2
/******************************************************************************
*
* lld_SectorEraseCmd - Writes a Sector Erase Command to Flash Device
*
* This function only issues the Sector Erase Command sequence.
* Erase status polling is not implemented in this function.
*
* RETURNS: void
*
*/
void lld_SectorEraseCmd
(
FLASHDATA * base_addr,                   /* device base address in system */
ADDRESS offset                           /* address offset from base address */
)
{
  /* Issue Sector Erase Command Sequence */
  FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR1, NOR_ERASE_SETUP_CMD);
  FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR2, NOR_SECTOR_ERASE_CMD);
}
#endif //LLD_SECTOR_ERASE_CMD_2
#ifdef LLD_CHIP_ERASE_CMD_1
/******************************************************************************
*
* lld_ChipEraseCmd - Writes a Chip Erase Command to Flash Device
*
* This function only issues the Chip Erase Command sequence.
* Erase status polling is not implemented in this function.
*
* RETURNS: void
*
*/
void lld_ChipEraseCmd
(
FLASHDATA * base_addr    /* device base address in system */
)
{
  /* Issue inlock sequence command */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_ERASE_SETUP_CMD);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  /* Write Chip Erase Command to Base Address */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_CHIP_ERASE_CMD);
}
#endif //LLD_CHIP_ERASE_CMD_1
#ifdef LLD_CHIP_ERASE_CMD_2
/******************************************************************************
*
* lld_ChipEraseCmd - Writes a Chip Erase Command to Flash Device
*
* This function only issues the Chip Erase Command sequence.
* Erase status polling is not implemented in this function.
*
* RETURNS: void
*
*/
void lld_ChipEraseCmd
(
FLASHDATA * base_addr    /* device base address in system */
)
{
  /* Issue Chip Erase Command Sequence */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_ERASE_SETUP_CMD);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_CHIP_ERASE_CMD);
}
#endif //LLD_CHIP_ERASE_CMD_2
#ifdef LLD_PROGRAM_CMD_1
/******************************************************************************
*
* lld_ProgramCmd - Writes a Program Command to Flash Device
*
* This function only issues the Program Command sequence.
* Program status polling is not implemented in this function.
*
* RETURNS: void
*
*/
void lld_ProgramCmd
(
FLASHDATA * base_addr,               /* device base address in system */
ADDRESS offset,                  /* address offset from base address */
FLASHDATA *pgm_data_ptr          /* variable containing data to program */
)
{
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  /* Write Program Command */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_PROGRAM_CMD);
  /* Write Data */
  FLASH_WR(base_addr, offset, *pgm_data_ptr);
}
#endif // LLD_PROGRAM_CMD_1
#ifdef LLD_PROGRAM_CMD_2
/******************************************************************************
*
* lld_ProgramCmd - Writes a Program Command to Flash Device
*
* This function issues the Program Command sequence.
* Program status polling can be conducted by polling either status register or toggling.
*
* RETURNS: void
*
*/

void lld_ProgramCmd
(
FLASHDATA * base_addr,               /* device base address in system */
ADDRESS offset,                  /* address offset from base address */
FLASHDATA *pgm_data_ptr          /* variable containing data to program */
)
{
  lld_WriteBufferProgramOp( base_addr, offset, 1, pgm_data_ptr);
}
#endif // #ifdef LLD_PROGRAM_CMD_2
#ifdef LLD_WRITE_BUFFER_CMD_1
/******************************************************************************
*
* lld_WriteToBufferCmd - Writes "Write to Buffer Pgm" Command sequence to Flash
*
* RETURNS: void
*
*/
void lld_WriteToBufferCmd
(
FLASHDATA * base_addr,               /* device base address in system */
ADDRESS offset                       /* address offset from base address */
)
{
  /* Issue unlock command sequence */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  /* Write Write Buffer Load Command */
  FLASH_WR(base_addr, offset, NOR_WRITE_BUFFER_LOAD_CMD);

#if (defined S29GLxxxP)
  FLSTATEOP = FLSTATE_WRITEBUFFER;
  g_writebuffer_sector = offset / LLD_SECTOR_SIZE;
#endif
}
/******************************************************************************
*
* lld_ProgramBufferToFlashCmd - Writes "Pgm Buffer To Flash" Cmd sequence to Flash
*
* RETURNS: void
*
*/
void lld_ProgramBufferToFlashCmd
(
FLASHDATA * base_addr,               /* device base address in system */
ADDRESS offset                       /* address offset from base address */
)
{
  /* Transfer Buffer to Flash Command */
  FLASH_WR(base_addr, offset, NOR_WRITE_BUFFER_PGM_CONFIRM_CMD);
}
/******************************************************************************
*
* lld_WriteBufferAbortResetCmd - Writes "Write To Buffer Abort" Reset to Flash
*
* RETURNS: void
*
*/
void lld_WriteBufferAbortResetCmd
(
FLASHDATA * base_addr        /* device base address in system */
)
{

  /* Issue Write Buffer Abort Reset Command Sequence */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  /* Write to Buffer Abort Reset Command */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_WRITE_BUFFER_ABORT_RESET_CMD);
}
#endif // LLD_WRITE_BUFFER_CMD_1
#ifdef LLD_WRITE_BUFFER_CMD_2
/******************************************************************************
*
* lld_WriteToBufferCmd - Writes "Write to Buffer Pgm" Command sequence to Flash
*
* RETURNS: void
*
*/
void lld_WriteToBufferCmd
(
FLASHDATA * base_addr,               /* device base address in system */
ADDRESS offset                       /* address offset from base address */
)
{
  FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR1, NOR_WRITE_BUFFER_LOAD_CMD);
}
/******************************************************************************
*
* lld_ProgramBufferToFlashCmd - Writes "Pgm Buffer To Flash" Cmd sequence to Flash
*
* RETURNS: void
*
*/
void lld_ProgramBufferToFlashCmd
(
FLASHDATA * base_addr,               /* device base address in system */
ADDRESS offset                       /* address offset from base address */
)
{
  /* Transfer Buffer to Flash Command */
  FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR1, NOR_WRITE_BUFFER_PGM_CONFIRM_CMD);
}
#endif // LLD_WRITE_BUFFER_CMD_2
#ifdef LLD_CFI_CMD_1
/******************************************************************************
*
* lld_CfiEntryCmd - Writes CFI Entry Command Sequence to Flash
*
* RETURNS: void
*
*/
void lld_CfiEntryCmd
(
FLASHDATA * base_addr /* device base address in system */
)
{

  FLASH_WR(base_addr, LLD_CFI_UNLOCK_ADDR1, NOR_CFI_QUERY_CMD);
}
/******************************************************************************
*
* lld_CfiExitCmd - Writes Cfi Exit Command Sequence to Flash
*
* This function resets the device out of CFI Query mode.
* This is a "wrapper function" to provide "Enter/Exit" symmetry in
* higher software layers.
*
* RETURNS: void
*
*/

void lld_CfiExitCmd
(
FLASHDATA * base_addr   /* device base address in system */
)
{

  /* Write Software RESET command */
  FLASH_WR(base_addr, 0, NOR_RESET_CMD);

}

/******************************************************************************
*
* lld_ReadCfiWord - Read CFI word operation.
*
* RETURNS: word read
*
*/
FLASHDATA lld_ReadCfiWord
(
FLASHDATA * base_addr,    /* device base address is system */
ADDRESS offset        /* address offset from base address */
)
{
  FLASHDATA data;

  lld_CfiEntryCmd(base_addr);
  data  = FLASH_RD(base_addr, offset);
  lld_CfiExitCmd(base_addr);

  return(data);
}

#endif // LLD_CFI_CMD_1

#ifdef LLD_CFI_CMD_2
/******************************************************************************
*
* lld_CfiEntryCmd - Writes CFI Entry Command Sequence to Flash
*
* RETURNS: void
*
*/
void lld_CfiEntryCmd
(
FLASHDATA * base_addr,    /* device base address in system */
ADDRESS offset            /* sector address offset for ASO(Address Space Overlay) */
)
{
  FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_CFI_UNLOCK_ADDR1, NOR_CFI_QUERY_CMD);
}
/******************************************************************************
*
* lld_CfiExitCmd - Writes Cfi Exit Command Sequence to Flash
*
* This function resets the device out of CFI Query mode.
* This is a "wrapper function" to provide "Enter/Exit" symmetry in
* higher software layers.
*
* RETURNS: void
*
*/

void lld_CfiExitCmd
(
FLASHDATA * base_addr   /* device base address in system */
)
{

  /* Write Software RESET command */
  FLASH_WR(base_addr, 0, NOR_RESET_CMD);

}

/******************************************************************************
*
* lld_ReadCfiWord - Read CFI word operation.
*
* RETURNS: word read
*
*/
FLASHDATA lld_ReadCfiWord
(
FLASHDATA * base_addr,    /* device base address is system */
ADDRESS offset        /* address offset from base address */
)
{
  FLASHDATA data;

  lld_CfiEntryCmd(base_addr, offset);
  data  = FLASH_RD(base_addr, offset);
  lld_CfiExitCmd(base_addr);

  return(data);
}

#endif // LLD_CFI_CMD_2

#ifdef LLD_STATUS_REG
/******************************************************************************
*
* lld_StatusRegReadCmd - Status register read command
*
* This function sends the status register read command before actually reading it.
*
* RETURNS: void
*
*/

#ifdef LLD_SIMULTANEOUS_OP_NONE
void lld_StatusRegReadCmd
(
FLASHDATA * base_addr    /* device base address in system */
)
{
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_STATUS_REG_READ_CMD);
}
#else
void lld_StatusRegReadCmd
(
FLASHDATA * base_addr,    /* device base address in system */
ADDRESS offset           /* address offset from base address */
)
{
  FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR1, NOR_STATUS_REG_READ_CMD);
}
#endif

/******************************************************************************
*
* lld_StatusRegClearCmd - Status register clear command
*
* This function clear the status register. It will not clear the device operation
* bits such as program suspend and erase suspend bits.
*
* RETURNS: void
*
*/

#ifdef LLD_SIMULTANEOUS_OP_NONE
void lld_StatusRegClearCmd
(
FLASHDATA * base_addr   /* device base address in system */
)
{
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_STATUS_REG_CLEAR_CMD);
}
#else
void lld_StatusRegClearCmd
(
FLASHDATA * base_addr,   /* device base address in system */
ADDRESS offset           /* sector address offset from base address */
)
{
  FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR1, NOR_STATUS_REG_CLEAR_CMD);
}
#endif
#endif		/* LLD_STATUS_REG */

#ifdef LLD_UNLOCKBYPASS_CMD
/******************************************************************************
*
* lld_UnlockBypassEntryCmd - Writes Unlock Bypass Enter command to flash
*
* RETURNS: void
*
*/
void lld_UnlockBypassEntryCmd
(
FLASHDATA * base_addr                /* device base address in system */
)
{

  /* Issue Unlock Bypass Enter Command Sequence */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_BYPASS_ENTRY_CMD);

}

/******************************************************************************
*
* lld_UnlockBypassProgramCmd - Writes Unlock Bypass Pgm Sequence to Flash
*
* This function issues the Unlock Bypass Programming Sequence to device.
* Device must be in Unlock Bypass mode before using this function.
* Status polling is not implemented in this function.
*
* RETURNS: void
*
*/
void lld_UnlockBypassProgramCmd
(
FLASHDATA * base_addr,           /* device base address in system */
ADDRESS offset,                  /* address offset from base address */
FLASHDATA *pgm_data_ptr          /* variable containing data to program */
)
{

  /* Issue Unlock Bypass Program Command Sequence */
  /* Write Program Command */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_PROGRAM_CMD);
  /* Write Data */
  FLASH_WR(base_addr, offset, *pgm_data_ptr);

}

/******************************************************************************
*
* lld_UnlockBypassResetCmd - Writes Unlock Bypass Exit command to flash
*
* Issues Unlock Bypass Exit command sequence to Flash Device.
*
* RETURNS: void
*
*/
void lld_UnlockBypassResetCmd
(
FLASHDATA * base_addr               /* device base address in system */
)
{
  /* Issue Unlock Bypass Exit Command Sequence */
  /* First Unlock Bypass Reset Command */
  FLASH_WR(base_addr, 0, NOR_UNLOCK_BYPASS_RESET_CMD1);
  /* Second Unlock Bypass Reset Command */
  FLASH_WR(base_addr, 0, NOR_UNLOCK_BYPASS_RESET_CMD2);
}
#endif /* LLD_UNLOCKBYPASS_CMD */

#ifdef LLD_AUTOSELECT_CMD
/******************************************************************************
*
* lld_AutoselectEntryCmd - Writes Autoselect Command Sequence to Flash
*
* This function issues the Autoselect Command Sequence to device.
*
* RETURNS: void
*
*/
void lld_AutoselectEntryCmd
(
FLASHDATA * base_addr           /* device base address in system */
)
{
  /* Issue Autoselect Command Sequence */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_AUTOSELECT_CMD);
}

/******************************************************************************
*
* lld_AutoselectExitCmd - Writes Autoselect Exit Command Sequence to Flash
*
* This function resets the device out of Autoselect mode.
* This is a "wrapper function" to provide "Enter/Exit" symmetry in
* higher software layers.
*
* RETURNS: void
*
*/
void lld_AutoselectExitCmd
(
FLASHDATA * base_addr			/* device base address in system */
)
{
  /* Write Software RESET command */
  FLASH_WR(base_addr, 0, NOR_RESET_CMD);
}
#endif /* LLD_AUTOSELECT_CMD */

#ifdef LLD_AUTOSELECT_CMD_2
/******************************************************************************
*
* lld_AutoselectEntryCmd - Writes Autoselect Command Sequence to Flash
*
* This function issues the Autoselect Command Sequence to device.
*
* RETURNS: void
*
*/
void lld_AutoselectEntryCmd
(
FLASHDATA * base_addr,			/* device base address in system */
ADDRESS offset					/* address offset from base address */
)
{
  /* Issue Autoselect Command Sequence */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR1, NOR_AUTOSELECT_CMD);
}

/******************************************************************************
*
* lld_AutoselectExitCmd - Writes Autoselect Exit Command Sequence to Flash
*
* This function resets the device out of Autoselect mode.
* This is a "wrapper function" to provide "Enter/Exit" symmetry in
* higher software layers.
*
* RETURNS: void
*
*/
void lld_AutoselectExitCmd
(
FLASHDATA * base_addr			/* device base address in system */
)
{
  /* Write Software RESET command */
  FLASH_WR(base_addr, 0, NOR_RESET_CMD);
}
#endif /* LLD_AUTOSELECT_CMD_2 */

#ifdef LLD_SUSP_RESUME_CMD_1
/******************************************************************************
*
* lld_ProgramSuspendCmd - Writes Suspend Command to Flash
*
* RETURNS: void
*
*/
void lld_ProgramSuspendCmd
(
FLASHDATA * base_addr,         /* device base address in system    */
ADDRESS offset                 /* address offset from base address */
)
{
  /* Write Suspend Command */
  FLASH_WR(base_addr, offset, NOR_SUSPEND_CMD);
}

/******************************************************************************
*
* lld_EraseSuspendCmd - Writes Suspend Command to Flash
*
* RETURNS: void
*
*/
void lld_EraseSuspendCmd
(
FLASHDATA * base_addr,       /* device base address in system    */
ADDRESS offset               /* address offset from base address */
)
{

#ifdef PAUSE_BETWEEN_ERASE_SUSPENDS
  DelayMilliseconds(10);
#endif

  #if (defined S29GLxxxP)
	FLASHDATA dq6_toggles, dq2_toggles;
	FLASHDATA status_read_1, status_read_2, status_read_3;

	status_read_1 = FLASH_RD(base_addr, offset);
	status_read_2 = FLASH_RD(base_addr, offset);
    status_read_3 = FLASH_RD(base_addr, offset);

	/* Any DQ6 toggles */
	dq6_toggles = ((status_read_1 ^ status_read_2) &        /* Toggles between read1 and read2 */
		           (status_read_2 ^ status_read_3) &        /* Toggles between read2 and read3 */
				   DQ6_MASK );                              /* Check for DQ6 only */

	if (dq6_toggles && (g_suspend_state == FLRESUME))
	{
		g_suspend_state = FLSUSPEND;
		g_suspend_sector = offset / LLD_SECTOR_SIZE;
	}
#endif

  /* Write Suspend Command */
  FLASH_WR(base_addr, offset, NOR_SUSPEND_CMD);
}

/******************************************************************************
*
* lld_EraseResumeCmd - Writes Resume Command to Flash
*
* RETURNS: void
*
*/
void lld_EraseResumeCmd
(
FLASHDATA * base_addr,       /* device base address in system    */
ADDRESS offset               /* address offset from base address */
)
{

  /* Write Resume Command */
  FLASH_WR(base_addr, offset, NOR_RESUME_CMD);

#if (defined S29GLxxxP)
	g_suspend_state = FLRESUME;
#endif
}

/******************************************************************************
*
* lld_ProgramResumeCmd - Writes Resume Command to Flash
*
* RETURNS: void
*
*/
void lld_ProgramResumeCmd
(
FLASHDATA * base_addr,       /* device base address in system    */
ADDRESS offset               /* address offset from base address */
)
{

  /* Write Resume Command */
  FLASH_WR(base_addr, offset, NOR_RESUME_CMD);

}
#endif /* LLD_SUSP_RESUME_CMD_1 */

#ifdef LLD_SUSP_RESUME_CMD_2
/******************************************************************************
*
* lld_ProgramSuspendCmd - Writes Suspend Command to Flash
*
* RETURNS: void
*
*/
void lld_ProgramSuspendCmd
(
FLASHDATA * base_addr           /* device base address in system */
)
{
  /* Write Suspend Command */
  FLASH_WR(base_addr, 0, NOR_PROGRAM_SUSPEND_CMD);
}

/******************************************************************************
*
* lld_EraseSuspendCmd - Writes Suspend Command to Flash
*
* RETURNS: void
*
*/
void lld_EraseSuspendCmd
(
FLASHDATA * base_addr      /* device base address in system */
)
{

  /* Write Suspend Command */
  FLASH_WR(base_addr, 0, NOR_ERASE_SUSPEND_CMD);
}

/******************************************************************************
*
* lld_EraseResumeCmd - Writes Resume Command to Flash
*
* RETURNS: void
*
*/
void lld_EraseResumeCmd
(
FLASHDATA * base_addr       /* device base address in system */
)
{

  /* Write Resume Command */
  FLASH_WR(base_addr, 0, NOR_ERASE_RESUME_CMD);

}

/******************************************************************************
*
* lld_ProgramResumeCmd - Writes Resume Command to Flash
*
* RETURNS: void
*
*/
void lld_ProgramResumeCmd
(
FLASHDATA * base_addr       /* device base address in system */
)
{
  /* Write Resume Command */
  FLASH_WR(base_addr, 0, NOR_PROGRAM_RESUME_CMD);
}

#endif /* LLD_SUSP_RESUME_CMD_2 */

#ifdef LLD_SUSP_RESUME_OP_1
/******************************************************************************
*
* lld_EraseSuspendOp - Performs Erase Suspend Operation
*
* Function performs erase suspend operation.
* Function issues erase suspend commands and will poll for completion.
*
* RETURNS: DEVSTATUS
*          DEV_ERASE_SUSPEND: Suspension OP is successful.
*          DEV_ERASE_SUSPEND_ERROR; Suspension OP fails. If Erase OP is complete,
*                           The function will return DEV_ERASE_SUSPEND_ERROR.
*/
DEVSTATUS lld_EraseSuspendOp
(
FLASHDATA * base_addr,		/* device base address is system */
ADDRESS offset              /* address offset from base address */
)
{
  FLASHDATA    status_reg;
  unsigned long	polling_counter = 0xFFFFFFFF;

#if (defined LLD_POLL_STATUS)
  lld_EraseSuspendCmd(base_addr);			/* issue erase suspend command */
  status_reg = lld_Poll(base_addr, 0);		/* wait for device done */
  if( (status_reg & DEV_ERASE_SUSP_MASK) == DEV_ERASE_SUSP_MASK )
	  return( DEV_ERASE_SUSPEND  );	       /* Erase suspend  */
  return( DEV_ERASE_SUSPEND_ERROR );       /* Erase suspend error */
#elif (defined LLD_POLL_TOGGLE)
  lld_EraseSuspendCmd(base_addr, offset);	/* issue erase suspend command */
  do {
	  polling_counter--;
  status_reg = lld_StatusGetReg(base_addr, offset);
  } while((status_reg == DEV_BUSY) && polling_counter);
  if( (status_reg & DEV_ERASE_SUSP_MASK) == DEV_ERASE_SUSP_MASK )
	  return( DEV_ERASE_SUSPEND  );	       /* Erase suspend  */
  return( DEV_ERASE_SUSPEND_ERROR );       /* Erase suspend error */
#else
  DelayMicroseconds(30);
  return( DEV_ERASE_SUSPEND  );	       /* Erase suspend  */
#endif
}

/******************************************************************************
*
* lld_ProgramSuspendOp - Performs Program Suspend Operation
*
* Function pergorm program suspend operation.
* Function issues program suspend commands and will poll for completion.
*
* RETURNS: DEVSTATUS
           DEV_PROGRAM_SUSPEND: Program Suspension OP is successful.
           DEV_PROGRAM_SUSPEND_ERROR: Program Suspension OP fails.
                If Program OP is complete, this function will return
                DEV_PROGRAM_SUSPEND_ERROR.
*/
DEVSTATUS lld_ProgramSuspendOp
(
FLASHDATA * base_addr,		/* Device base address is system */
ADDRESS offset              /* address offset from base address */
)
{
  FLASHDATA    status_reg;
  unsigned long	polling_counter = 0xFFFFFFFF;

#if (defined LLD_POLL_STATUS)
  lld_ProgramSuspendCmd(base_addr);			/* Issue program suspend command */
  status_reg = lld_Poll( base_addr, 0 );
  if( (status_reg & DEV_PROGRAM_SUSP_MASK) == DEV_PROGRAM_SUSP_MASK )
	  return( DEV_PROGRAM_SUSPEND );	    /* Program suspend */
  return( DEV_PROGRAM_SUSPEND_ERROR  );	    /* Program suspend error */
#elif (defined LLD_POLL_TOGGLE)
  lld_ProgramSuspendCmd(base_addr, offset);	/* Issue program suspend command */
  do {
	  polling_counter--;
  status_reg = lld_StatusGetReg(base_addr, offset);
  } while((status_reg == DEV_BUSY) && polling_counter);
  if( (status_reg & DEV_PROGRAM_SUSP_MASK) == DEV_PROGRAM_SUSP_MASK )
	  return( DEV_PROGRAM_SUSPEND );	    /* Program suspend */
  return( DEV_PROGRAM_SUSPEND_ERROR  );	    /* Program suspend error */
#else
  DelayMicroseconds(30);
  return( DEV_PROGRAM_SUSPEND  );	       /* Erase suspend  */
#endif
}
#endif /* LLD_SUSP_RESUME_OP_1 */

#ifdef LLD_SUSP_RESUME_OP_2
/******************************************************************************
*
* lld_EraseSuspendOp - Performs Erase Suspend Operation
*
* Function pergorm erase suspend operation.
* Function issues erase suspend commands and will poll for completion.
*
* RETURNS: DEVSTATUS
*/
DEVSTATUS lld_EraseSuspendOp
(
FLASHDATA * base_addr    /* device base address is system */
)
{
#ifdef LLD_POLL_STATUS
    FLASHDATA    status_reg;
#endif

  lld_EraseSuspendCmd( base_addr );		/* issue erase suspend command */
#ifdef LLD_POLL_STATUS
  status_reg = lld_Poll(base_addr, 0 );    /* wait for device done */

  if( (status_reg & DEV_ERASE_SUSP_MASK) == DEV_ERASE_SUSP_MASK )
	  return( DEV_ERASE_SUSPEND  );	       /* Erase suspend  */

  return( DEV_ERASE_SUSPEND_ERROR );       /* Erase suspend error */
#else
  DelayMicroseconds(30);
  return( DEV_ERASE_SUSPEND  );	       /* Erase suspend  */
#endif

}

/******************************************************************************
*
* lld_ProgramSuspendOp - Performs Program Suspend Operation
*
* Function pergorm program suspend operation.
* Function issues program suspend commands and will poll for completion.
*
* RETURNS: DEVSTATUS
*/
DEVSTATUS lld_ProgramSuspendOp
(
FLASHDATA * base_addr    /* Device base address is system */
)
{
#ifdef LLD_POLL_STATUS
    FLASHDATA    status_reg;
#endif

  lld_ProgramSuspendCmd( base_addr );		/* Issue program suspend command */
#ifdef LLD_POLL_STATUS
  status_reg = lld_Poll( base_addr, 0 );

  if( (status_reg & DEV_PROGRAM_SUSP_MASK) == DEV_PROGRAM_SUSP_MASK )
	  return( DEV_PROGRAM_SUSPEND );	    /* Program suspend */

  return( DEV_PROGRAM_SUSPEND_ERROR  );	    /* Program suspend error */
#else
  DelayMicroseconds(30);
  return( DEV_PROGRAM_SUSPEND  );	       /* Erase suspend  */
#endif
}
#endif /* LLD_SUSP_RESUME_OP_2 */

#ifdef LLD_GET_ID_CMD_1
/******************************************************************************
*
* lld_GetDeviceId - Get device ID operation
*
* RETURNS: three byte ID in a single int
*
*/
unsigned int lld_GetDeviceId
(
FLASHDATA * base_addr   /* device base address in system */
)
{
  unsigned int id;


  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_AUTOSELECT_CMD);

#if (defined LLD_CONFIGURATION_X8X16_AS_X8) || (defined LLD_CONFIGURATION_X8_AS_X8)
  id  = (unsigned int)(FLASH_RD(base_addr, 0x0002) & 0x000000FF) << 16;
  id |= (unsigned int)(FLASH_RD(base_addr, 0x001C) & 0x000000FF) <<  8;
  id |= (unsigned int)(FLASH_RD(base_addr, 0x001E) & 0x000000FF)      ;
#else
  id  = (unsigned int)(FLASH_RD(base_addr, 0x0001) & 0x000000FF) << 16;
  id |= (unsigned int)(FLASH_RD(base_addr, 0x000E) & 0x000000FF) <<  8;
  id |= (unsigned int)(FLASH_RD(base_addr, 0x000F) & 0x000000FF)      ;
#endif
  /* Write Software RESET command */
  FLASH_WR(base_addr, 0, NOR_RESET_CMD);

  return(id);
}
#endif /* LLD_GET_ID_CMD_1 */

#ifdef LLD_GET_ID_CMD_2
/******************************************************************************
*
* lld_GetDeviceId - Get device ID operation
*
* RETURNS: three byte ID in a single int
*
*/
unsigned int lld_GetDeviceId
(
FLASHDATA * base_addr,   /* device base address in system */
ADDRESS offset           /* ASO(Address Space Overlay) */
)
{
  unsigned int id;

  /* CFI entry command */
  FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR1, NOR_CFI_QUERY_CMD);

#if (defined LLD_CONFIGURATION_X8X16_AS_X8) || (defined LLD_CONFIGURATION_X8_AS_X8)
  id  = (unsigned int)(FLASH_RD(base_addr+offset, 0x0002) & 0x000000FF) << 16;
  id |= (unsigned int)(FLASH_RD(base_addr+offset, 0x001C) & 0x000000FF) <<  8;
  id |= (unsigned int)(FLASH_RD(base_addr+offset, 0x001E) & 0x000000FF)      ;
#else
  id  = (unsigned int)(FLASH_RD(base_addr+offset, 0x0001) & 0x000000FF) << 16;
  id |= (unsigned int)(FLASH_RD(base_addr+offset, 0x000E) & 0x000000FF) <<  8;
  id |= (unsigned int)(FLASH_RD(base_addr+offset, 0x000F) & 0x000000FF)      ;
#endif

  /* Write Software RESET command */
  FLASH_WR(base_addr, 0, NOR_RESET_CMD);

  return(id);
}
#endif  /* LLD_GET_ID_CMD_2 */

#ifdef LLD_CONFIG_REG_CMD_1
/******************************************************************************
*
* lld_SetConfigRegCmd - Set configuration register command
*
* RETURNS: n/a
*/

void lld_SetConfigRegCmd
(
FLASHDATA *   base_addr,	/* device base address in system */
FLASHDATA value
)
{
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_SET_CONFIG_CMD);

  FLASH_WR(base_addr, 0, value);

}

/******************************************************************************
*
* lld_ReadConfigRegCmd - Read configuration register command.
*
* RETURNS: config register
*
*/
FLASHDATA lld_ReadConfigRegCmd
(
FLASHDATA *   base_addr		/* device base address in system */
)
{
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_READ_CONFIG_CMD);
  return(FLASH_RD(base_addr, 0));

}
#endif /* LLD_CONFIG_REG_CMD_1 */

#ifdef LLD_CONFIG_REG_CMD_2
/******************************************************************************
*
* lld_SetConfigRegCmd - Set configuration register command
*
* RETURNS: n/a
*/
void lld_SetConfigRegCmd
(
FLASHDATA *   base_addr,        /* device base address */
ADDRESS offset,					/* sector address offset for ASO(Address Space Overlay) */
FLASHDATA value
)
{
  /* Write buffer command with one word to config register */
  FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR1, NOR_SET_CONFIG_CMD);
  /* write to buffer command */
  FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR1, NOR_WRITE_BUFFER_LOAD_CMD);

  FLASH_WR( base_addr, LLD_UNLOCK_ADDR2, 0);             /* word count */
  FLASH_WR( base_addr, 0, value );                       /* write the config register */
  /* Transfer Buffer to Flash Command */
  FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR1, NOR_WRITE_BUFFER_PGM_CONFIRM_CMD);
}

/******************************************************************************
*
* lld_ReadConfigRegCmd
*
* RETURNS: config register
*
*/
FLASHDATA lld_ReadConfigRegCmd
(
FLASHDATA *   base_addr,
ADDRESS offset
)
{
  FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR1, NOR_SET_CONFIG_CMD);
  return( FLASH_RD(base_addr, (offset & SA_OFFSET_MASK)) );		/* read the config. reg. */
}
#endif /* LLD_CONFIG_REG_CMD_2 */

#ifdef LLD_CONFIG_REG_CMD_3 /* For WS-P device Configuration Reg. 1 */
/******************************************************************************
*
* lld_SetConfigRegCmd - Set configuration register command for WS-P device.
* WS-P type device uses config. reg. 1 to set wait state.
* RETURNS: n/a
*/

void lld_SetConfigRegCmd
(
FLASHDATA *   base_addr,	/* device base address in system */
FLASHDATA value,			/* Configuration Register 0 value */
FLASHDATA value1			/* Configuration Register 1 value */
)
{

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_SET_CONFIG_CMD);

  FLASH_WR(base_addr, 0, value);		/* configuration register 0 */
  FLASH_WR(base_addr, 1, value1);       /* configuration register 1 */

}

/******************************************************************************
*
* lld_ReadConfigRegCmd - Read configuration register command.
*
* RETURNS: config register
*
*/
FLASHDATA lld_ReadConfigRegCmd
(
FLASHDATA *   base_addr,	/* device base address in system */
FLASHDATA offset			/* configuration reg. offset 0/1 */
)
{

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_READ_CONFIG_CMD);
  return(FLASH_RD(base_addr, offset));

}
#endif /* LLD_CONFIG_REG_CMD_3 */

#ifdef LLD_POLL_TOGGLE
#ifdef LLD_DEV_FLASH
/*************************************************************************
*
* lld_Poll - Polls flash device for embedded operation completion
*
* Function polls the Flash device to determine when an embedded
*  operation has finished.
*  Function performs a verify on the polling address if the device
*  returns from status routine in a non-busy/non-error state.
*
* <polling_type> variable is used to determine type of polling to perform.
*  This variable tells the status routine what mode the device is in.
*  Future versions of this function will also use the <polling_type>
*  variable to properly initialize and implement watchdog timers.
*  Acceptable values for <polling_type> are:
*   LLD_P_POLL_PGM
*   LLD_P_POLL_WRT_BUF_PGM
*   LLD_P_POLL_SEC_ERS
*   LLD_P_POLL_CHIP_ERS
*
* RETURNS: DEVSTATUS
*
*/
DEVSTATUS lld_Poll
(
FLASHDATA * base_addr,          /* device base address in system */
ADDRESS offset,                 /* address offset from base address */
FLASHDATA *exp_data_ptr,        /* expect data */
FLASHDATA *act_data_ptr,        /* actual data */
POLLING_TYPE polling_type       /* type of polling to perform */
)
{
  DEVSTATUS       dev_status;
  unsigned long   polling_counter = 0xFFFFFFFF;


  do
  {
    polling_counter--;
    dev_status = lld_StatusGet(base_addr, offset);
  }
  while((dev_status == DEV_BUSY) && polling_counter);

  /* if device returns status other than "not busy" then we
     have a polling error state. */
  if(dev_status != DEV_NOT_BUSY)
  {
#ifdef LLD_WRITE_BUFFER_OP
    if(dev_status == DEV_WRITE_BUFFER_ABORT)
    {
      lld_WriteBufferAbortResetCmd(base_addr);
    }
    else
    {
      /* Write Software RESET command */
      FLASH_WR(base_addr, 0, NOR_RESET_CMD);
    }
#endif
    /* indicate to caller that there was an error */
    return(dev_status);
  }
  else
  {
	 /* read the actual data */
	 *act_data_ptr = FLASH_RD(base_addr, offset);

	 /* Check that polling location verifies correctly */
	 if((*exp_data_ptr & LLD_DB_READ_MASK) == (*act_data_ptr & LLD_DB_READ_MASK))
	 {
	   dev_status = DEV_NOT_BUSY;
	   return(dev_status);
	 }
	 else
	 {
	   dev_status = DEV_VERIFY_ERROR;
	   return(dev_status);
	 }
  }
}
#endif /* LLD_DEV_FLASH */
#endif /* LLD_POLL_TOGGLE */

#ifdef LLD_POLL_TOGGLE_AS_STATUS_API
/******************************************************************************
*
* lld_StatusEmulateReg - translates legacy toggle bits to status register data
*
*  Function reads DQ6 to determine whether the device is busy or ready.
*
* RETURNS: value of status register
*
*/
#define CHECK_TOGGLE_BIT_ERRORS
#define DQ1_MASK   (0x02 * LLD_DEV_MULTIPLIER)  /* DQ1 mask for all interleave devices */
#define DQ3_MASK   (0x08 * LLD_DEV_MULTIPLIER)  /* DQ3 mask for all interleave devices */
#define DQ5_MASK   (0x20 * LLD_DEV_MULTIPLIER)  /* DQ5 mask for all interleave devices */
#define DQ6_MASK   (0x40 * LLD_DEV_MULTIPLIER)  /* DQ6 mask for all interleave devices */
#define DQ6_TGL_RDY_SHFT (dq6_toggles << 1) /* Shift for DQ6 to ready bit position in status register */
#define DQ6_TGL_DQ1_MASK (dq6_toggles >> 5) /* Mask for DQ1 when device DQ6 toggling */
#define DQ6_TGL_DQ5_MASK (dq6_toggles >> 1) /* Mask for DQ5 when device DQ6 toggling */
FLASHDATA lld_StatusEmulateReg
(
FLASHDATA * base_addr   /* device base address in system */
)
{
  /* Note:
  *     Since this function always checks status at the start of the flash
  *     (offset zero), this routine cannot be used with banked devices.
  *     Since DQ2 behaves differently for reads in the erasing sector, it
  *     cannot be used to detect erase suspend, nor erase vs. program.
  *     This routine should not be called between suspending and resuming.
  *     It is only valid to call this code before suspend or after resume.
  *     Since this routine has no knowledge of the current operation
  *     (program or erase), it cannot detect write buffer abort.  Also, if
  *     this function detected a DQ5 error, it would not be clear whether
  *     to report a program error or erase error.  Therefore, in the event
  *     of an error, this routine will continually return busy, and it is
  *     the responsibility of the calling code to time out.
  */
  FLASHDATA dq6_toggles;
  FLASHDATA status_read_1;
  FLASHDATA status_read_2;
  FLASHDATA status_read_3;
  FLASHDATA status_reg;

  status_read_1 = FLASH_RD(base_addr, 0);
  status_read_2 = FLASH_RD(base_addr, 0);
  status_read_3 = FLASH_RD(base_addr, 0);

  /* Any DQ6 toggles */
  dq6_toggles = ((status_read_1 ^ status_read_2) &          /* Toggles between read1 and read2 */
                  (status_read_2 ^ status_read_3) &         /* Toggles between read2 and read3 */
                  DQ6_MASK );                               /* Check for DQ6 only */

#ifdef CHECK_TOGGLE_BIT_ERRORS
  if (dq6_toggles)
  {
	if ((DQ6_TGL_DQ5_MASK & status_read_1) == DQ6_TGL_DQ5_MASK )
    {
      FLASH_WR (base_addr, 0, 0x0b0b);
      LLD_PRINTF ("Timeout: status read 1 = %d, status read 2 = %d status read 3 = %d\n", status_read_1, status_read_2, status_read_3);
      return (DEV_RDY_MASK | DEV_ERASE_MASK);
    }
  }
#endif  /* CHECK_TOGGLE_BIT_ERRORS */

  /* Move DQ6 indicator to ready bit position */
  status_reg = DQ6_TGL_RDY_SHFT;

  /* Translate DQ6 toggle indicator (1 = busy) to ready bit (1 = ready) */
  status_reg = status_reg ^ DEV_RDY_MASK;

  return status_reg;
}
#endif

#ifdef LLD_POLL_STATUS
#ifdef LLD_DEV_FLASH
/******************************************************************************
*
* lld_Poll - Polls flash device for embedded operation completion
*
*  Function polls the Flash device to determine when an embedded
*  operation has finished - bit 7 is 1.
*
* RETURNS: value of status register
*
*/
FLASHDATA lld_Poll
(
FLASHDATA * base_addr,          /* device base address in system */
ADDRESS offset                 /* address offset from base address */
)
{
  unsigned long polling_counter = 0xFFFFFFFF;
  volatile FLASHDATA status_reg;

  do
  {
    polling_counter--;
#ifdef LLD_SIMULTANEOUS_OP_NONE
	lld_StatusRegReadCmd( base_addr );		/* Issue status register read command */
#else
    lld_StatusRegReadCmd( base_addr, offset );		/* Issue status register read command */
#endif
    status_reg = FLASH_RD(base_addr, offset);       /* read the status register */
	if( (status_reg & DEV_RDY_MASK) == DEV_RDY_MASK  )  /* Are all devices done bit 7 is 1 */
		break;

  }while(polling_counter);

  return( status_reg );          /* retrun the status reg. */
}
#endif /* LLD_DEV_FLASH */
#endif /* LLD_POLL_STATUS */


#ifdef LLD_STATUS_REG
#ifdef LLD_STATUS_GET
/******************************************************************************
*
* lld_StatusGet - Determines Flash operation status
*
*
* RETURNS: DEVSTATUS
*
*/
DEVSTATUS lld_StatusGet
(
FLASHDATA *  base_addr,      /* device base address in system */
ADDRESS      offset          /* address offset from base address */
)
{
    FLASHDATA status_reg;

    lld_StatusRegReadCmd( base_addr, offset );		/* Issue status register read command */
    status_reg = FLASH_RD(base_addr, offset);       /* read the status register */

    if( (status_reg & DEV_SEC_LOCK_MASK) == DEV_SEC_LOCK_MASK )
	  return( DEV_SECTOR_LOCK );
    else if( (status_reg & DEV_ERASE_SUSP_MASK) == DEV_ERASE_SUSP_MASK )
	  return( DEV_ERASE_SUSPEND );
    else if( (status_reg & DEV_ERASE_MASK) == DEV_ERASE_MASK )
	  return( DEV_ERASE_ERROR );
    else if( (status_reg & DEV_PROGRAM_SUSP_MASK) == DEV_PROGRAM_SUSP_MASK )
	  return( DEV_PROGRAM_SUSPEND );
    else if( (status_reg & DEV_PROGRAM_MASK) == DEV_PROGRAM_MASK )
	  return( DEV_PROGRAM_ERROR );
    else if( (status_reg & DEV_RDY_MASK) == DEV_RDY_MASK  )
	   return( DEV_NOT_BUSY );

    return( DEV_BUSY );
}
#endif
/******************************************************************************
*
* lld_StatusClear - Clears the flash status
*
*
* RETURNS: void
*
*/
#ifdef LLD_SIMULTANEOUS_OP_NONE
void lld_StatusClear
(
FLASHDATA *  base_addr      /* device base address in system */
)
{
    lld_StatusRegClearCmd(base_addr );
}
#else
void lld_StatusClear
(
FLASHDATA *  base_addr,      /* device base address in system */
ADDRESS      offset          /* address offset from base address */
)
{
    lld_StatusRegClearCmd(base_addr, offset );
}
#endif

/******************************************************************************
*
* lld_StatusGetReg - Gets the flash status register bits
*
*
* RETURNS: FLASHDATA
*
*/
FLASHDATA lld_StatusGetReg
(
FLASHDATA *  base_addr,      /* device base address in system */
ADDRESS      offset          /* address offset from base address */
)
{
    FLASHDATA status_reg = 0xFFFF;
#ifdef LLD_SIMULTANEOUS_OP_NONE
	lld_StatusRegReadCmd( base_addr );		/* Issue status register read command */
#else
    lld_StatusRegReadCmd( base_addr, offset );		/* Issue status register read command */
#endif
    status_reg = FLASH_RD( base_addr, offset );     /* read the status register */
    return status_reg;
}

#else // No LLD_STATUS_REG
#ifdef LLD_STATUS_GET
/******************************************************************************
*
* lld_StatusGet - Determines Flash operation status
*
*
* RETURNS: DEVSTATUS
*
*/

DEVSTATUS lld_StatusGet
(
FLASHDATA *  base_addr,      /* device base address in system */
ADDRESS      offset          /* address offset from base address */
)
{

	FLASHDATA dq2_toggles;
	FLASHDATA dq6_toggles;
	FLASHDATA status_read_1;
	FLASHDATA status_read_2;
	FLASHDATA status_read_3;

	status_read_1 = FLASH_RD(base_addr, offset);
	status_read_2 = FLASH_RD(base_addr, offset);
    status_read_3 = FLASH_RD(base_addr, offset);

	/* Any DQ6 toggles */
	dq6_toggles = ((status_read_1 ^ status_read_2) &        /* Toggles between read1 and read2 */
		           (status_read_2 ^ status_read_3) &        /* Toggles between read2 and read3 */
				   DQ6_MASK );                              /* Check for DQ6 only */

	/* Any DQ2 toggles */
    dq2_toggles = ((status_read_1 ^ status_read_2) &        /* Toggles between read1 and read2 */
		           (status_read_2 ^ status_read_3) &        /* Toggles between read2 and read3 */
				   DQ2_MASK );                              /* Check for DQ6 only */

	if (dq6_toggles)
	{
		/* Checking WriteBuffer Abort condition:
		   Check for all devices that have DQ6 toggling also have Write Buffer Abort DQ1 set */
		if ((!dq2_toggles) &&
			((DQ6_TGL_DQ1_MASK & status_read_1) == DQ6_TGL_DQ1_MASK)
#if (defined S29GLxxxP)
			 && (FLSTATEOP == FLSTATE_WRITEBUFFER)
			 && (g_writebuffer_sector == (offset / LLD_SECTOR_SIZE))
#endif
			 )
			return DEV_WRITE_BUFFER_ABORT;

		/* Checking Timeout condition:
		   Check for all devices that have DQ6 toggling also have Time Out DQ5 set. */
		if ((DQ6_TGL_DQ5_MASK & status_read_1) == DQ6_TGL_DQ5_MASK )
		   return DEV_EXCEEDED_TIME_LIMITS;

		/* No timeout, no WB error */
		return DEV_BUSY;
	}
	else   /* no DQ6 toggles on all devices */
	{
		/* Checking Erase Suspend condition */
		status_read_1 = FLASH_RD(base_addr, offset);
		status_read_2 = FLASH_RD(base_addr, offset);

		/* Checking Erase Suspend condition */
        if ((((status_read_1 ^ status_read_2) & DQ2_MASK) == 0)
#if (defined S29GLxxxP)
 			 && (g_suspend_state == FLRESUME)
#endif
			)
		   return DEV_NOT_BUSY;         /* All devices DQ2 not toggling */
		else							// at least one device is suspended
		{
#if (defined S29GLxxxP)
			if ( (g_suspend_state == FLSUSPEND) &&
				 (g_suspend_sector != (offset / LLD_SECTOR_SIZE))
			   )
				return DEV_NOT_BUSY;     /* At least some devices toggle DQ2 */
			else
#endif
		   return DEV_SUSPEND;          /* At least some devices toggle DQ2 */
	}
	}
}

/******************************************************************************
*
* lld_StatusClear - Clears the flash status
*
*
* RETURNS: void
*
*/
void lld_StatusClear
(
FLASHDATA *  base_addr,      /* device base address in system */
ADDRESS      offset          /* address offset from base address */
)
{
    FLASHDATA status_reg;

    status_reg = lld_StatusGetReg(base_addr, offset);

    /*WBA happened */
    if( ((status_reg & DEV_RDY_MASK) == DEV_RDY_MASK)  &&          /* device is ready */
        ((status_reg & DEV_PROGRAM_MASK) == DEV_PROGRAM_MASK) && /* program error */
        ((status_reg & DEV_ERASE_MASK) != DEV_ERASE_MASK)        /* no erase error */
      )
    {
#ifdef LLD_WRITE_BUFFER_CMD_1
        lld_WriteBufferAbortResetCmd(base_addr);
#endif
    }
    else
    {
        lld_ResetCmd(base_addr);
    }
}


/******************************************************************************
*
* lld_StatusGetReg - Gets the flash status register bits
*
*
* RETURNS: FLASHDATA
*
*/
FLASHDATA lld_StatusGetReg
(
FLASHDATA *  base_addr,      /* device base address in system */
ADDRESS      offset          /* address offset from base address */
)
{
    FLASHDATA sector_protected = 0x0;
    FLASHDATA status_reg = 0x0000;
    DEVSTATUS status;

   	status = lld_StatusGet(base_addr, offset);

    if(status == DEV_NOT_BUSY)
    {
        status_reg |= DEV_RDY_MASK;
    }
    else if(status == DEV_WRITE_BUFFER_ABORT) /* Can happen only because of failed program op*/
    {
        status_reg |= DEV_RDY_MASK;
        status_reg |= DEV_PROGRAM_MASK;
    }
    else if(status == DEV_EXCEEDED_TIME_LIMITS) /* Can happen either by failed program or erase op */
    {
        status_reg |= DEV_RDY_MASK;
        status_reg |= DEV_PROGRAM_MASK;
        status_reg |= DEV_ERASE_MASK;
    }
    else if(status == DEV_SUSPEND) /* Only erase suspend can be detected by DQ polling */
    {
        status_reg |= DEV_RDY_MASK;
        status_reg |= DEV_ERASE_SUSP_MASK;
    }

    if((status != DEV_BUSY) && (status != DEV_WRITE_BUFFER_ABORT))
    {
    #ifdef LLD_AUTOSELECT_CMD
    /* find if sectior protected by reading autoselct offset 0x02*/
      lld_AutoselectEntryCmd(base_addr);

      offset &= ~(0xFF);
	  #if (defined LLD_CONFIGURATION_X8X16_AS_X8) || (defined LLD_CONFIGURATION_X8_AS_X8)
		offset |= 0x04;
	  #else
		offset |= 0x02;
	  #endif
      sector_protected = (FLASH_RD(base_addr, offset));

      lld_AutoselectExitCmd(base_addr);
    #endif
    }

    if(sector_protected == 0x1)
    {
        status_reg |= DEV_SEC_LOCK_MASK;
    }

    return status_reg;
}

#endif /// LLD_STATUS_GET

#endif // LLD_STATUS_REG

#ifdef LLD_SECSI_SECTOR_CMD_1
/******************************************************************************
*
* lld_SecSiSectorExitCmd - Writes SecSi Sector Exit Command Sequence to Flash
*
* This function issues the Secsi Sector Exit Command Sequence to device.
* Use this function to Exit the SecSi Sector.
*
*
* RETURNS: void
*
* ERRNO:
*/

void lld_SecSiSectorExitCmd
(
FLASHDATA * base_addr               /* device base address in system */
)
{
  /* Issue SecSi Sector Exit Command Sequence */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  /* First Secsi Sector Reset Command */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_SECSI_SECTOR_EXIT_SETUP_CMD);
  /* Second Secsi Sector Reset Command */
  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_CMD);
}
/******************************************************************************
*
* lld_SecSiSectorEntryCmd - Writes SecSi Sector Entry Command Sequence to Flash
*
* This function issues the Secsi Sector Entry Command Sequence to device.
* Use this function to Enable the SecSi Sector.
*
*
* RETURNS: void
*
* ERRNO:
*/
void lld_SecSiSectorEntryCmd
(
FLASHDATA * base_addr    /* device base address in system */
)
{

  /* Issue SecSi Sector Entry Command Sequence */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_SECSI_SECTOR_ENTRY_CMD);
}
#endif /* LLD_SECSI_SECTOR_CMD_1 */

#ifdef LLD_SECSI_SECTOR_CMD_2
/******************************************************************************
*
* lld_SecSiSectorExitCmd - Writes SecSi Sector Exit Command Sequence to Flash
*
* This function issues the Secsi Sector Exit Command Sequence to device.
* Use this function to Exit the SecSi Sector.
*
*
* RETURNS: void
*
* ERRNO:
*/
void lld_SecSiSectorExitCmd
(
FLASHDATA * base_addr               /* device base address in system */
)
{
  /* Issue SecSi Sector Exit Command Sequence */
  FLASH_WR(base_addr, 0, NOR_RESET_CMD);
}

/******************************************************************************
*
* lld_SecSiSectorEntryCmd - Writes SecSi Sector Entry Command Sequence to Flash
*
* This function issues the Secsi Sector Entry Command Sequence to device.
* Use this function to Enable the SecSi Sector.
*
*
* RETURNS: void
*
* ERRNO:
*/
void lld_SecSiSectorEntryCmd
(
FLASHDATA * base_addr,		/* device base address in system */
ADDRESS offset				/* sector offset for ASO(Address Space Overlay) */
)
{
  /* Issue SecSi Sector Entry Command Sequence */
  FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR1, NOR_SECSI_SECTOR_ENTRY_CMD);
}
#endif /* LLD_SECSI_SECTOR_CMD_2 */


#ifdef LLD_SECSI_SECTOR_CMD_3
/******************************************************************************
*
* lld_SecSiSectorExitCmd - Writes SecSi Sector Exit Command Sequence to Flash
*
* This function issues the Secsi Sector Exit Command Sequence to device.
* Use this function to Exit the SecSi Sector.
*
*
* RETURNS: void
*
* ERRNO:
*/

void lld_SecSiSectorExitCmd
(
FLASHDATA * base_addr               /* device base address in system */
)
{
  /* Issue SecSi Sector Exit Command Sequence */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  /* First Secsi Sector Reset Command */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_SECSI_SECTOR_EXIT_SETUP_CMD);
  /* Second Secsi Sector Reset Command */
  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_CMD);
}
/******************************************************************************
*
* lld_SecSiSectorEntryCmd - Writes SecSi Sector Entry Command Sequence to Flash
*
* This function issues the Secsi Sector Entry Command Sequence to device.
* Use this function to Enable the SecSi Sector.
*
*
* RETURNS: void
*
* ERRNO:
*/
void lld_SecSiSectorEntryCmd
(
FLASHDATA * base_addr,      /* device base address in system */
ADDRESS offset				/* sector offset for ASO(Address Space Overlay) */
)
{

  /* Issue SecSi Sector Entry Command Sequence */
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR1, NOR_SECSI_SECTOR_ENTRY_CMD);
}
#endif /* LLD_SECSI_SECTOR_CMD_3 */


#ifdef LLD_BLANK_CHK_CMD
/******************************************************************************
*
* lld_BlankCheckCmd - Blank Check command
*
* This function checks the sector is blank.
*
* RETURNS: void
*
* ERRNO:
*/
void lld_BlankCheckCmd
(
FLASHDATA * base_addr,   /* device base address in system */
ADDRESS offset           /* sector address offset from base address */
)
{
  FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR1, NOR_BLANK_CHECK_CMD);
}
#endif /* LLD_BLANK_CHK_CMD */

#ifdef LLD_BLANK_CHK_OP
/******************************************************************************
*
* lld_BlankCheckOp - Performs a Blank Check Operation
*
* Function check blank at <base_addr> + <offset>.
* Function issues all required commands and will pool for completion
*
* RETURNS: DEVSTATUS
*/
DEVSTATUS lld_BlankCheckOp
(
FLASHDATA * base_addr,    /* device base address is system */
ADDRESS offset        /* address offset from base address */
)
{
#ifdef LLD_POLL_STATUS
    FLASHDATA    status_reg;
#elif defined LLD_STATUS_GET
    DEVSTATUS    dev_status;
#endif

  unsigned long  polling_counter = 0xFFFFFFFF;

  lld_BlankCheckCmd( base_addr, offset );

#ifdef LLD_POLL_STATUS
  status_reg = lld_Poll(base_addr, offset );

  if( (status_reg & DEV_ERASE_MASK) == DEV_ERASE_MASK )
	  return( DEV_ERASE_ERROR );		/* sector not blank */
  else
	   return( DEV_NOT_BUSY );			/* sector are blank */

#elif defined LLD_STATUS_GET
    dev_status = DEV_STATUS_UNKNOWN;

	/* poll for completion */
	do
	{
		polling_counter--;
		/* no checking for interrupts */

		dev_status = lld_StatusGet((FLASHDATA *)base_addr, 0);

	} while ((dev_status == DEV_BUSY) && polling_counter);

    /* if not done, then we have an error */
    if (dev_status != DEV_NOT_BUSY)
    {
       /* Write Software RESET command */
       FLASH_WR(base_addr, 0, NOR_RESET_CMD);
       return (DEV_ERASE_ERROR); /* return error */
    }
	else
	{
		return( DEV_NOT_BUSY );			/* sector are blank */
	}
#endif
}
#endif /* LLD_LLD_BLANK_CHK_OP */


#ifdef LLD_READ_OP
/******************************************************************************
*
* lld_ReadOp - Read memory array operation
*
* RETURNS: data read
*
*/
FLASHDATA lld_ReadOp
(
FLASHDATA * base_addr,    /* device base address is system */
ADDRESS offset        /* address offset from base address */
)
{
  FLASHDATA data;

  data = FLASH_RD(base_addr, offset);

  return(data);
}
#endif /* LLD_READ_OP */

#ifdef LLD_WRITE_BUFFER_OP
/******************************************************************************
*
* lld_WriteBufferProgramOp - Performs a Write Buffer Programming Operation.
*
* Function programs a write-buffer overlay of addresses to data
* passed via <data_buf>.
* Function issues all required commands and polls for completion.
*
* There are 4 main sections to the function:
*  Set-up and write command sequence
*  Determine number of locations to program and load buffer
*  Start operation with "Program Buffer to Flash" command
*  Poll for completion
*
* REQUIREMENTS:
*  Data in the buffer MUST be properly aligned with the Flash bus width.
*  No parameter checking is performed.
*  The <word_count> variable must be properly initialized.
*  Valid <byte_cnt> values:
*   min = 1 byte (only valid when used with a single x8 Flash)
*   max = write buffer size in bytes * number of devices in parallel
      (e.g. 32-byte buffer per device, 2 x16 devices = 64 bytes)
*
* RETURNS: DEVSTATUS
*/
#ifdef LLD_STATUS_REG
DEVSTATUS lld_WriteBufferProgramOp
(
FLASHDATA *   base_addr,  /* device base address in system     */
ADDRESS   offset,     /* address offset from base address  */
WORDCOUNT word_count, /* number of words to program        */
FLASHDATA *data_buf   /* buffer containing data to program */
)
{
  ADDRESS   last_loaded_addr;
  ADDRESS   current_offset;
  ADDRESS   end_offset;
  FLASHDATA wcount;
  FLASHDATA status_reg;

  /* Initialize variables */
  current_offset   = offset;
  end_offset       = offset + word_count - 1;
  last_loaded_addr = offset;

  /* don't try with a count of zero */
  if (!word_count)
  {
    return(DEV_NOT_BUSY);
  }

  /* Issue Load Write Buffer Command Sequence */
  lld_WriteToBufferCmd(base_addr, offset);

  /* Write # of locations to program */
  wcount = (FLASHDATA)word_count - 1;
  wcount *= LLD_DEV_MULTIPLIER;			/* For interleaving devices */

  /* In the datasheets of some latest Spansion devices, such as GLP, GLS, etc, the
  command sequence of "write to buffer" command states the address of word count is
  "Sector Address". Notice that to make LLD backward compatibility, the actual word
  count address implemented is "Sector Address + LLD_UNLOCK_ADDR2", since the lower
  address bits (a0-a15) are "don't care" bits and will be ignored anyway.
  */
  FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR2 , wcount);

  /* Load Data into Buffer */
  while(current_offset <= end_offset)
  {
    /* Store last loaded address & data value (for polling) */
    last_loaded_addr = current_offset;

    /* Write Data */
    FLASH_WR(base_addr, current_offset, *data_buf++);
    current_offset++;
  }

  /* Issue Program Buffer to Flash command */
  lld_ProgramBufferToFlashCmd(base_addr, offset);

  status_reg = lld_Poll(base_addr, last_loaded_addr);

  if( status_reg & DEV_SEC_LOCK_MASK )
	  return( DEV_SECTOR_LOCK );		/* sector locked */

  if( (status_reg & DEV_PROGRAM_MASK) == DEV_PROGRAM_MASK )
	  return( DEV_PROGRAM_ERROR );		/* program error */

  return( DEV_NOT_BUSY );			    /* program complete */
}
#else	/* toggle loop */
DEVSTATUS lld_WriteBufferProgramOp
(
FLASHDATA *   base_addr,  /* device base address in system     */
ADDRESS   offset,     /* address offset from base address  */
WORDCOUNT word_count, /* number of words to program        */
FLASHDATA *data_buf   /* buffer containing data to program */
)
{
  DEVSTATUS status;
  FLASHDATA write_data = 0;
  FLASHDATA read_data = 0;
  ADDRESS   last_loaded_addr;
  ADDRESS   current_offset;
  ADDRESS   end_offset;
  FLASHDATA wcount;

  /* Initialize variables */
  current_offset   = offset;
  end_offset       = offset + word_count - 1;
  last_loaded_addr = offset;

  /* don't try with a count of zero */
  if (!word_count)
  {
    return(DEV_NOT_BUSY);
  }

  /* Issue Load Write Buffer Command Sequence */
  lld_WriteToBufferCmd(base_addr, offset);

  /* Write # of locations to program */
  wcount = (FLASHDATA)word_count - 1;
  wcount *= LLD_DEV_MULTIPLIER;

  FLASH_WR(base_addr, offset, wcount);

  /* Load Data into Buffer */
  while(current_offset <= end_offset)
  {
    /* Store last loaded address & data value (for polling) */
    last_loaded_addr = current_offset;
    write_data = *data_buf;

    /* Write Data */
    FLASH_WR(base_addr, current_offset++, *data_buf++);
  }

  /* Issue Program Buffer to Flash command */
  lld_ProgramBufferToFlashCmd(base_addr, last_loaded_addr);

  status = lld_Poll(base_addr, last_loaded_addr, &write_data,
                    &read_data, LLD_P_POLL_WRT_BUF_PGM);
  return(status);
}
#endif /* LLD_STATUS_REG */
#endif /* LLD_WRITE_BUFFER_OP */

#ifdef LLD_PROGRAM_OP
/******************************************************************************
*
* lld_ProgramOp - Performs a standard Programming Operation.
*
* Function programs a single location to the specified data.
* Function issues all required commands and polls for completion.
*
*
* RETURNS: DEVSTATUS
*/
#ifdef LLD_STATUS_REG
DEVSTATUS lld_ProgramOp
(
FLASHDATA * base_addr,      /* device base address is system */
ADDRESS offset,         /* address offset from base address */
FLASHDATA write_data    /* variable containing data to program */
)
{
 DEVSTATUS status;
 status = lld_WriteBufferProgramOp(base_addr, offset, 1, &write_data );
 return(status);
}
#else
DEVSTATUS lld_ProgramOp
(
FLASHDATA * base_addr,      /* device base address is system */
ADDRESS offset,         /* address offset from base address */
FLASHDATA write_data    /* variable containing data to program */
)
{
  FLASHDATA read_data = 0;
  DEVSTATUS status;

  lld_ProgramCmd(base_addr, offset, &write_data);
  status = lld_Poll(base_addr, offset, &write_data, &read_data, LLD_P_POLL_PGM);
  return(status);
}
#endif
#endif /* LLD_PROGRAM_OP */

#ifdef LLD_SECTOR_ERASE_OP
/******************************************************************************
*
* lld_SectorEraseOp - Performs a Sector Erase Operation
*
* Function erases the sector containing <base_addr> + <offset>.
* Function issues all required commands and polls for completion.
*
*
* RETURNS: DEVSTATUS
*
* ERRNO:
*/
#ifdef LLD_STATUS_REG

#ifdef LLD_DEV_FLASH
DEVSTATUS lld_SectorEraseOp
(
FLASHDATA * base_addr,    /* device base address is system */
ADDRESS offset        /* address offset from base address */
)
{
  FLASHDATA         status_reg;

  lld_SectorEraseCmd(base_addr, offset);
  status_reg = lld_Poll(base_addr, offset );

  if( status_reg & DEV_SEC_LOCK_MASK )
	  return( DEV_SECTOR_LOCK );		/* sector locked */

  if( (status_reg & DEV_ERASE_MASK) == DEV_ERASE_MASK )
	  return( DEV_ERASE_ERROR );		/* erase error */

  return( DEV_NOT_BUSY );			    /* erease complete */
}
#endif  /* LLD_DEV_FLASH */
#else
#ifdef LLD_DEV_FLASH
DEVSTATUS lld_SectorEraseOp
(
FLASHDATA * base_addr,    /* device base address is system */
ADDRESS offset        /* address offset from base address */
)
{
  FLASHDATA         expect_data = (FLASHDATA)0xFFFFFFFF;
  FLASHDATA         actual_data = 0;
  DEVSTATUS         status;

  lld_SectorEraseCmd(base_addr, offset);

  status = lld_Poll(base_addr, offset, &expect_data, &actual_data, LLD_P_POLL_SEC_ERS);
  return(status);
}
#endif /* LLD_DEV_FLASH */
#endif /* LLD_STATUS_REG */

#endif /* LLD_SECTOR_ERASE_OP */

#ifdef LLD_CHIP_ERASE_OP
/******************************************************************************
*
* lld_ChipEraseOp - Performs a Chip Erase Operation
*
* Function erases entire device located at <base_addr>.
* Function issues all required commands and polls for completion.
*
*
* RETURNS: DEVSTATUS
*/
#ifdef LLD_STATUS_REG
DEVSTATUS lld_ChipEraseOp
(
FLASHDATA * base_addr   /* device base address in system */
)
{
  FLASHDATA status_reg;

  lld_ChipEraseCmd(base_addr);
  status_reg = lld_Poll(base_addr, 0 );

  if( (status_reg & DEV_ERASE_MASK) == DEV_ERASE_MASK )
	  return( DEV_ERASE_ERROR );		/* erase error */

  return( DEV_NOT_BUSY );			    /* erease complete */
}
#else
DEVSTATUS lld_ChipEraseOp
(
FLASHDATA * base_addr   /* device base address in system */
)
{

  DEVSTATUS status;
  FLASHDATA expect_data = (FLASHDATA)0xFFFFFFFF;
  FLASHDATA actual_data = 0;

  lld_ChipEraseCmd(base_addr);
  status = lld_Poll(base_addr, 0, &expect_data, &actual_data, LLD_P_POLL_CHIP_ERS);

  /* if an error during polling, write RESET command to device */
  if(status != DEV_NOT_BUSY)
	/* Write Software RESET command */
	FLASH_WR(base_addr, 0, NOR_RESET_CMD);
  return(status);
}

#endif
#endif /* LLD_CHIP_ERASE_OP */

#ifdef LLD_MEMCPY_OP
/******************************************************************************
*
* lld_memcpy   This function attempts to mimic the standard memcpy
*              function for flash.  It segments the source data
*              into page size chunks for use by Write Buffer Programming.
*
* RETURNS: DEVSTATUS
*
*/
DEVSTATUS lld_memcpy
(
FLASHDATA * base_addr,    /* device base address is system */
ADDRESS offset,           /* address offset from base address */
WORDCOUNT word_cnt,       /* number of words to program */
FLASHDATA *data_buf       /* buffer containing data to program */
)
{
  ADDRESS mask = LLD_BUFFER_SIZE - 1;
  WORDCOUNT intwc = word_cnt;
  DEVSTATUS status = DEV_NOT_BUSY;

  if (offset & mask)
  {
     /* program only as much as necessary, so pick the lower of the two numbers */
     if (word_cnt < (LLD_BUFFER_SIZE - (offset & mask)) )
        intwc = word_cnt;
     else
        intwc = LLD_BUFFER_SIZE - (offset & mask);

     /* program the first few to get write buffer aligned */
     status = lld_WriteBufferProgramOp(base_addr, offset, intwc, data_buf);
     if (status != DEV_NOT_BUSY)
     {
       return(status);
     }

     offset   += intwc; /* adjust pointers and counter */
     word_cnt -= intwc;
     data_buf += intwc;
     if (word_cnt == 0)
     {
       return(status);
     }
  }

  while(word_cnt >= LLD_BUFFER_SIZE) /* while big chunks to do */
  {
    status = lld_WriteBufferProgramOp(base_addr, offset, LLD_BUFFER_SIZE, data_buf);
    if (status != DEV_NOT_BUSY)
    {
      return(status);
    }

    offset   += LLD_BUFFER_SIZE; /* adjust pointers and counter */
    word_cnt -= LLD_BUFFER_SIZE;
    data_buf += LLD_BUFFER_SIZE;
  }
  if (word_cnt == 0)
  {
    return(status);
  }

  status = lld_WriteBufferProgramOp(base_addr, offset, word_cnt, data_buf);
  return(status);
}
#endif /* LLD_MEMCPY_OP */

#ifdef LLD_DELAY_MSEC
/******************************************************************************
*
* DelayMilliseconds - Performs a delay.  If you have a better way,
*                     edit the macro DELAY_MS in lld_target_specific.h
*
* RETURNS: void
*
*/
void DelayMilliseconds(int milliseconds)
{
  int i;

  for (i = 0; i < milliseconds; i++)
    DelayMicroseconds(1000);

}

/******************************************************************************
*
* DelayMicroseconds - Performs a delay.  If you have a better way,
*
* RETURNS: void
*
*/
#define DELAY_1us 3

void DelayMicroseconds(int microseconds)
{
  int volatile i, j;

  for (j = 0; j < microseconds; j++)
    for(i = 0; i < DELAY_1us; i++) ;

}
#endif /* LLD_DELAY_MSEC */

/************************************************************
*************************************************************
* Following are sector protection driver routines			*
*************************************************************
*************************************************************/

#ifdef LLD_LOCKREG2_ENTRY
/******************************************************************************
*
* lld_LockRegEntryCmd - Lock register entry command.
*
* RETURNS: n/a
*
*/
void lld_LockReg2EntryCmd
(
FLASHDATA *   base_addr		/* device base address in system */
)
{

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, WSXXX_LOCK_REG2_ENTRY);
}
#endif

#ifdef LLD_LOCKREG_CMD_1
/******************************************************************************
*
* lld_LockRegEntryCmd - Lock register entry command.
*
* RETURNS: n/a
*
*/
void lld_LockRegEntryCmd
(
FLASHDATA *   base_addr		/* device base address in system */
)
{

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, WSXXX_LOCK_REG_ENTRY);
}

/******************************************************************************
*
* lld_LockRegBitsProgramCmd - Lock register program command.
*
* RETURNS: n/a
*
*/
void lld_LockRegBitsProgramCmd
(
FLASHDATA *   base_addr,	/* device base address in system */
FLASHDATA value				/* value to program to lock reg. */
)
{

  FLASH_WR(base_addr, 0, NOR_UNLOCK_BYPASS_PROGRAM_CMD);
  FLASH_WR(base_addr, 0x0, value);

}

/******************************************************************************
*
* lld_LockRegBitsReadCmd - Lock register read command.
* Note: Need to issue lld_LockRegEntryCmd() before use this routine.
*
* RETURNS:
* DQ0 Customer SecSi Sector Protection Bit  0 = set
* DQ1 Persistent Protection Mode Lock Bit   0 = set
* DQ2 Password Protection Mode Lock Bit     0 = set
*
*/
FLASHDATA lld_LockRegBitsReadCmd
(
FLASHDATA *   base_addr		/* device base address in system */
)
{

	return(FLASH_RD(base_addr, 0x0));

}

/******************************************************************************
*
* lld_LockRegExitCmd - Exit lock register read/write mode command.
*
* RETURNS: n/a
*
*/
void lld_LockRegExitCmd
(
FLASHDATA *   base_addr		/* device base address in system */
)
{

  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_SETUP_CMD);
  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_CMD);

}
#endif

#ifdef LLD_LOCKREG_CMD_2
/******************************************************************************
*
* lld_SSRLockRegEntryCmd: SSR(Secured Silicon Region) Lock Register entry command.
*
* RETURNS: n/a
*
*/
void lld_SSRLockRegEntryCmd
(
FLASHDATA *   base_addr,
ADDRESS offset				/* sector offset for ASO(Address Space Overlay) */
)
{
  FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR1, NOR_LOCK_REG_ENTRY);
}

/******************************************************************************
*
* lld_SSRLockRegBitsProgramCmd: SSR(Secured Silicon Region) Lock Register program command
*
* DQ0 Customer SSR(Secured Silicon Region) Protection Bit  1 = lock set
* DQ1 Factory SSR(Secured Silicon Region) Protection Bit  1 = lock set
*
* RETURNS: n/a
*
*/
void lld_SSRLockRegBitsProgramCmd
(
FLASHDATA *   base_addr,
ADDRESS offset,				/* sector offset for ASO(Address Space Overlay) */
FLASHDATA value
)
{
  lld_WriteToBufferCmd( base_addr, offset );			// write to buffer command
  FLASH_WR( base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR2, 0);			// word count
  FLASH_WR( base_addr, offset, value * LLD_DEV_MULTIPLIER );	// write the lock register
  lld_ProgramBufferToFlashCmd( base_addr, offset );			// write to flash cmd
}

/******************************************************************************
*
* lld_SSRLockRegBitsReadCmd: SSR(Secured Silicon Region) Lock Register read command
*
* RETURNS:
* DQ0 Customer SecSi Sector Protection Bit  1 = lock set
* DQ1 Factory SecSi Sector Protection Bit  1 = lock set
*
*/
FLASHDATA lld_SSRLockRegBitsReadCmd
(
FLASHDATA *   base_addr,
ADDRESS offset				/* sector offset for ASO(Address Space Overlay) */
)
{
	return(FLASH_RD(base_addr, offset));
}

/******************************************************************************
*
* lld_SSRLockRegExitCmd: SSR(Secured Silicon Region) lock register exit command
*
* RETURNS: n/a
*
*/
void lld_SSRLockRegExitCmd
(
FLASHDATA *   base_addr
)
{
  /* Write Software RESET command */
  FLASH_WR(base_addr, 0, NOR_RESET_CMD);
}
#endif /* LLD_LOCKREG_CMD */


#ifdef LLD_PASSWORD_CMD
/******************************************************************************
*
* lld_PasswordProtectionEntryCmd - Write Password Protection Entry command sequence
*
* RETURNS: n/a
*
*/
void lld_PasswordProtectionEntryCmd
(
FLASHDATA *   base_addr		/* device base address in system */
)
{

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, WSXXX_PSWD_PROT_CMD_ENTRY);
}

/******************************************************************************
*
* lld_PasswordProtectionProgramCmd - Write Password Protection Program command.
* Note: Need to issue lld_PasswordProtectionEntryCmd() before issue this routine.
* RETURNS: n/a
*
*/
void lld_PasswordProtectionProgramCmd
(
FLASHDATA *   base_addr,	/* device base address in system */
ADDRESS   offset,
FLASHDATA pwd
)
{
  FLASH_WR(base_addr, offset, NOR_UNLOCK_BYPASS_PROGRAM_CMD);
  FLASH_WR(base_addr, offset, pwd);
}

/******************************************************************************
*
* lld_PasswordProtectionReadCmd - Issue read password command
* Note: Need to issue lld_PasswordProtectionEntryCmd() before issue this routine.
* RETURNS: n/a
*
*/
void lld_PasswordProtectionReadCmd
(
FLASHDATA *   base_addr,	/* device base address in system */
FLASHDATA *pwd0,			/* Password 0 */
FLASHDATA *pwd1,			/* Password 1 */
FLASHDATA *pwd2,			/* Password 2 */
FLASHDATA *pwd3				/* Password 3 */
)
{
  *pwd0 = FLASH_RD(base_addr, 0);
  *pwd1 = FLASH_RD(base_addr, 1);
  *pwd2 = FLASH_RD(base_addr, 2);
  *pwd3 = FLASH_RD(base_addr, 3);
}

/******************************************************************************
*
* lld_PasswordProtectionUnlockCmd - Issue unlock password command.
* Note: Need to issue lld_PasswordProtectionEntryCmd() before issue this routine.
* RETURNS: n/a
*
*/
void lld_PasswordProtectionUnlockCmd
(
FLASHDATA *   base_addr,	/* device base address in system */
FLASHDATA pwd0,				/* Password 0 */
FLASHDATA pwd1,				/* Password 1 */
FLASHDATA pwd2,				/* Password 2 */
FLASHDATA pwd3				/* Password 3 */
)
{
  FLASH_WR(base_addr, 0, WSXXX_PSWD_UNLOCK_1);
  FLASH_WR(base_addr, 0, WSXXX_PSWD_UNLOCK_2);
  FLASH_WR(base_addr, 0, pwd0);
  FLASH_WR(base_addr, 1, pwd1);
  FLASH_WR(base_addr, 2, pwd2);
  FLASH_WR(base_addr, 3, pwd3);
  FLASH_WR(base_addr, 0, WSXXX_PSWD_UNLOCK_3);

}

/******************************************************************************
*
* lld_PasswordProtectionExitCmd - Issue exit password protection mode command.
*
* RETURNS: n/a
*
*/
void lld_PasswordProtectionExitCmd
(
FLASHDATA *   base_addr		/* device base address in system */
)
{
  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_SETUP_CMD);
  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_CMD);
}
#endif /* LLD_PASSWORD_CMD */

#ifdef LLD_PPB_CMD
/******************************************************************************
*
* lld_PpbEntryCmd - Non-Volatile Sector Protection Entry Command.
* Ppb entry command will disable the reads and writes for the bank selectd.
* RETURNS: n/a
*
*/
void lld_PpbEntryCmd
(
FLASHDATA *   base_addr,	/* device base address in system */
ADDRESS       offset		/* Sector offset address */
)
{

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1+(offset & SA_OFFSET_MASK), WSXXX_PPB_ENTRY);
}

/******************************************************************************
*
* lld_PpbProgramCmd - Program Non-Volatile Sector Protection Command.
* Note: Need to issue lld_PpbEntryCmd() before issue this routine.
* RETURNS: n/a
*
*/
void lld_PpbProgramCmd
(
FLASHDATA *   base_addr,	/* device base address in system */
ADDRESS   offset			/* Sector offset address */
)
{
  FLASH_WR(base_addr, 0, NOR_UNLOCK_BYPASS_PROGRAM_CMD);
  FLASH_WR(base_addr, offset, 0);
}

/******************************************************************************
*
* lld_PpbAllEraseCmd - Erase Non-Volatile Protection for All  Sectors Command.
* Note: Need to issue lld_PpbEntryCmd() before issue this routine.
* RETURNS: n/a
*
*/
void lld_PpbAllEraseCmd
(
FLASHDATA *   base_addr		/* device base address in system */
)
{
  FLASH_WR(base_addr, 0, NOR_ERASE_SETUP_CMD);
  FLASH_WR(base_addr, 0, WSXXX_PPB_ERASE_CONFIRM);
}

/******************************************************************************
*
* lld_PpbStatusReadCmd - Read Non-Volatile Sector Status Command.
* Note: Need to issue lld_PpbEntryCmd() before issue this routine.
* Sector status 0 is locked and 1 is unlocked.
* RETURNS:
*
*/
FLASHDATA lld_PpbStatusReadCmd
(
FLASHDATA *   base_addr,	/* device base address in system */
ADDRESS   offset			/* sector offset address */
)
{
  return(FLASH_RD(base_addr, offset));
}

/******************************************************************************
*
* lld_PpbExitCmd - Exit the Non-Volatile Sector Status mode.
* After the exit command the device goes back to memory array mode.
* RETURNS: n/a
*
*/
void lld_PpbExitCmd
(
FLASHDATA *   base_addr		/* device base address in system */
)
{
  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_SETUP_CMD);
  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_CMD);
}
#endif /* LLD_PPB_CMD */

#ifdef LLD_LOCKBIT_CMD
/******************************************************************************
*
* lld_PpbLockBitEntryCmd - Issue Persistent Protection Bit Lock Bit Entry Command.
* The Ppb Lock Bit is a global bit for all sectors.
* RETURNS: n/a
*
*/
void lld_PpbLockBitEntryCmd
(
FLASHDATA *   base_addr		/* device base address in system */
)
{

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, WSXXX_PPB_LOCK_ENTRY);
}

/******************************************************************************
*
* lld_PpbLockBitSetCmd - Issue set Persistent Protection Bit Lock Bit command.
* Once bit is set there is no command to unset it only hardware reset and power up
* will clear the bit.
* RETURNS: n/a
*
*/
void lld_PpbLockBitSetCmd
(
FLASHDATA *   base_addr		/* device base address in system */
)
{
  FLASH_WR(base_addr, 0, NOR_UNLOCK_BYPASS_PROGRAM_CMD);
  FLASH_WR(base_addr, 0, 0);
}

/******************************************************************************
*
* lld_PpbLockBitReadCmd - Read the Ppb Lock Bit value.
* Note: Need to issue lld_PpbLockBitEntryCmd() before read.
* RETURNS:
*
*/
FLASHDATA lld_PpbLockBitReadCmd
(
FLASHDATA *   base_addr		/* device base address in system */
)
{
  return(FLASH_RD(base_addr, 0));
}

/******************************************************************************
*
* lld_PpbLockBitExitCmd - Exit Ppb Lock Bit mode command.
*
* RETURNS: n/a
*
*/
void lld_PpbLockBitExitCmd
(
FLASHDATA *   base_addr		/* device base address in system */
)
{
  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_SETUP_CMD);
  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_CMD);
}
#endif /* LLD_LOCKBIT_CMD */


#ifdef LLD_DYB_CMD
/******************************************************************************
*
* lld_DybEntryCmd - Dynamic (Volatile) Sector Protection Entry Command.
*
* RETURNS: n/a
*
*/
void lld_DybEntryCmd
(
FLASHDATA *   base_addr		/* device base address in system */
)
{

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_UNLOCK_DATA1);
  FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_UNLOCK_DATA2);

  FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, WSXXX_DYB_ENTRY);
}

/******************************************************************************
*
* lld_DybSetCmd - Dynamic (Volatile) Sector Protection Set Command.
* Note: Need to issue lld_DybEntryCmd() before issue this command.
* RETURNS: n/a
*
*/
void lld_DybSetCmd
(
FLASHDATA *   base_addr,	/* device base address in system */
ADDRESS   offset			/* sector offset address */
)
{
  FLASH_WR(base_addr, 0, NOR_UNLOCK_BYPASS_PROGRAM_CMD);
  FLASH_WR(base_addr, offset, 0x00000000);
}

/******************************************************************************
*
* lld_DybClrCmd - Dynamic (Volatile) Sector Protection Clear Command.
* Note: Need to issue lld_DybEntryCmd() before issue this command.
* RETURNS: n/a
*
*/
void lld_DybClrCmd
(
FLASHDATA *   base_addr,	/* device base address in system */
ADDRESS   offset			/* sector offset address */
)
{
  FLASH_WR(base_addr, 0, NOR_UNLOCK_BYPASS_PROGRAM_CMD);
  FLASH_WR(base_addr, offset, WSXXX_DYB_CLEAR);
}

/******************************************************************************
*
* lld_DybReadCmd - Dynamic (Volatile) Sector Protection Read Command.
* Note: Need to issue lld_DybEntryCmd() before issue this command.
* RETURNS:
*
*/
FLASHDATA lld_DybReadCmd
(
FLASHDATA *   base_addr,	/* device base address in system */
ADDRESS   offset			/* sector offset address */
)
{
  return(FLASH_RD(base_addr, offset));
}

/******************************************************************************
*
* lld_DybExitCmd - Exit Dynamic (Volatile) Sector Protection Mode Command.
*
* RETURNS: n/a
*
*/
void lld_DybExitCmd
(
FLASHDATA *   base_addr		/* device base address in system */
)
{
  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_SETUP_CMD);
  FLASH_WR(base_addr, 0, NOR_SECSI_SECTOR_EXIT_CMD);
}
#endif /* LLD_DYB_CMD */

#ifdef LLD_LOCK_BIT_RD_OP
/******************************************************************************
*
* lld_PpbLockBitReadOp - Operation to read global lock bit.
*
* RETURNS: FLASHDATA
*
*/
FLASHDATA  lld_PpbLockBitReadOp ( FLASHDATA *base_addr)
{
  FLASHDATA retval;

  (lld_PpbLockBitEntryCmd)(base_addr);
  retval = (lld_PpbLockBitReadCmd)(base_addr);
  lld_PpbLockBitExitCmd(base_addr);

  return retval;
}
#endif /* LLD_LOCK_BIT_RD_OP */

#ifdef LLD_LOCK_BIT_SET_OP
/******************************************************************************
*
* lld_PpbLockBitSetOp - Operation to set the global lock bit.
*
* RETURNS: 0 Successful
*         -1 Failed
*
*/
int lld_PpbLockBitSetOp ( FLASHDATA *   base_addr)
{
  DEVSTATUS      dev_status = DEV_STATUS_UNKNOWN;
  unsigned long  polling_counter = 0xFFFFFFFF;
#ifdef LLD_POLL_STATUS
    FLASHDATA    status_reg;
#endif


    lld_PpbLockBitEntryCmd(base_addr);
    lld_PpbLockBitSetCmd(base_addr);
#ifdef LLD_POLL_STATUS
	/* poll for completion */
    status_reg = lld_Poll(base_addr, 0);
    if( (status_reg & DEV_PROGRAM_MASK) == DEV_PROGRAM_MASK )
	    dev_status =  DEV_PROGRAM_ERROR;		/* program error */
	else
		dev_status = DEV_NOT_BUSY;

#elif defined LLD_STATUS_GET
    /* poll for completion */
    do
    {
    	polling_counter--;
    	/* no checking for interrupts */

    	dev_status = lld_StatusGet((FLASHDATA *)base_addr, 0);

    } while ((dev_status == DEV_BUSY) && polling_counter);
#endif
    /* if not done, then we have an error */
    if (dev_status != DEV_NOT_BUSY)
    {
		/* Write Software RESET command */
		FLASH_WR(base_addr, 0, NOR_RESET_CMD);
        lld_PpbLockBitExitCmd(base_addr);
        return (-1);  /* return error */
    }

    lld_PpbLockBitExitCmd(base_addr);
    return 0; /* successfull */
}
#endif /* LLD_LOCK_BIT_SET_OP */

#ifdef LLD_PPB_ERASE_OP
/******************************************************************************
*
* lld_PpbAllEraseOp - Operation to clear protection for all sections.
*
* RETURNS: 0 Successful
*         -1 Failed
*
*/
int lld_PpbAllEraseOp ( FLASHDATA *   base_addr)
{
  DEVSTATUS    dev_status = DEV_STATUS_UNKNOWN;
  unsigned long  polling_counter = 0xFFFFFFFF;
#ifdef LLD_POLL_STATUS
    FLASHDATA    status_reg;
#endif

    if (lld_PpbLockBitReadOp(base_addr) == PPB_PROTECTED)    /* if it is already locked */
    {
       return(-1);                            /* return an error cuz Lock Bit is locked */
    }

    lld_PpbEntryCmd(base_addr, 0);
    lld_PpbAllEraseCmd(base_addr);
#ifdef LLD_POLL_STATUS
	/* poll for completion */
    status_reg = lld_Poll(base_addr, 0);
    if( (status_reg & DEV_PROGRAM_MASK) == DEV_PROGRAM_MASK )
	    dev_status =  DEV_PROGRAM_ERROR;		/* program error */
	else
		dev_status = DEV_NOT_BUSY;

#elif defined LLD_STATUS_GET

    /* poll for completion */
    do
    {
        polling_counter--;
        /* no checking for interrupts */

        dev_status = lld_StatusGet((FLASHDATA *)base_addr, 0);

    } while ((dev_status == DEV_BUSY) && polling_counter);
#endif
    /* if not done, then we have an error */
    if (dev_status != DEV_NOT_BUSY)
    {
        /* Write Software RESET command */
        FLASH_WR(base_addr, 0, NOR_RESET_CMD);
        lld_PpbExitCmd(base_addr);
        return (-1); /* return error */
    }

    lld_PpbExitCmd(base_addr);		/* exit Ppb protection mode command */
    return 0; /* successful */
}
#endif /* LLD_PPB_ERASE_OP */

#ifdef LLD_PPB_PGM_OP
/******************************************************************************
*
* lld_PpbProgramOp - Operation set the Persistent Protection for a sector.
*
* RETURNS: 0 Successful
*         -1 Failed
*
*/
int lld_PpbProgramOp ( FLASHDATA *base_addr, ADDRESS offset)
{
  DEVSTATUS    dev_status = DEV_STATUS_UNKNOWN;
  unsigned long  polling_counter = 0xFFFFFFFF;
#ifdef LLD_POLL_STATUS
    FLASHDATA    status_reg;
#endif

  if (lld_PpbLockBitReadOp(base_addr) == PPB_PROTECTED)      /* if it is already locked */
  {
     return(-1);                              /* return an error cuz Lock Bit is locked */
  }


  lld_PpbEntryCmd(base_addr,offset);
  lld_PpbProgramCmd(base_addr, offset);

#ifdef LLD_POLL_STATUS
	/* poll for completion */
    status_reg = lld_Poll(base_addr, 0);
    if( (status_reg & DEV_PROGRAM_MASK) == DEV_PROGRAM_MASK )
	    dev_status =  DEV_PROGRAM_ERROR;		/* program error */
	else
		dev_status = DEV_NOT_BUSY;

#elif defined LLD_STATUS_GET
  /* poll for completion */
  do
  {
     polling_counter--;
     /* no checking for interrupts */
     /* poll Sector 0 for completion - workaround */
     dev_status = lld_StatusGet((FLASHDATA *)base_addr, 0);

  } while ((dev_status == DEV_BUSY) && polling_counter);
#endif
  /* if not done, then we have an error */
  if (dev_status != DEV_NOT_BUSY)
  {
     /* Write Software RESET command */
     FLASH_WR(base_addr, 0, NOR_RESET_CMD);
     lld_PpbExitCmd(base_addr);
     return (-1); /* return error */
  }

  lld_PpbExitCmd(base_addr);
  return 0; /* successful */
}
#endif /* LLD_PPB_PGM_OP */

#ifdef LLD_PPB_READ_OP
/******************************************************************************
*
* lld_PpbStatusReadOp - Operation to read the Persistent Protection status register.
*
* RETURNS: FLASHDATA
*lld_read
*/

FLASHDATA lld_PpbStatusReadOp ( FLASHDATA *base_addr, ADDRESS offset)
{
   FLASHDATA  status;

   lld_PpbEntryCmd(base_addr,offset);
   status = (FLASH_RD(base_addr, offset));
   lld_PpbExitCmd(base_addr);

   return status;

}
#endif /* LLD_PPB_READ_OP */

#ifdef LLD_LOCKREG_RD_OP_1
/******************************************************************************
*
* lld_LockRegBitsReadOp - Operation to read the lock status register.
*
* RETURNS: FLASHDATA
*
*/
FLASHDATA lld_LockRegBitsReadOp ( FLASHDATA *base_addr)
{
  FLASHDATA value;

  lld_LockRegEntryCmd(base_addr);
  value = lld_LockRegBitsReadCmd(base_addr);
  lld_LockRegExitCmd(base_addr);

  return(value);
}
#endif

#ifdef LLD_LOCKREG_RD_OP_2
/******************************************************************************
*
* lld_SSRLockRegBitsReadOp: Operation to read SSR Lock Register
*
* RETURNS: FLASHDATA
*
*/
FLASHDATA lld_SSRLockRegBitsReadOp ( FLASHDATA *base_addr, ADDRESS offset)
{
  FLASHDATA value;

  lld_SSRLockRegEntryCmd(base_addr, offset);
  value = lld_SSRLockRegBitsReadCmd(base_addr, offset);
  lld_SSRLockRegExitCmd(base_addr);

  return(value);
}
#endif /* LLD_LOCKREG_RD_OP */

#ifdef LLD_LOCKREG_WR_OP_1
#ifdef LLD_DEV_FLASH
/******************************************************************************
*
* lld_LockRegBitsProgramOp - Operation to program the lock register.
*
* RETURNS: 0 Successful
*         -1 Failed
*
*/
int lld_LockRegBitsProgramOp ( FLASHDATA *base_addr, FLASHDATA value)
{
  DEVSTATUS    dev_status = DEV_STATUS_UNKNOWN;
  unsigned long  polling_counter = 0xFFFFFFFF;
#ifdef LLD_POLL_STATUS
    FLASHDATA    status_reg;
#endif

  lld_LockRegEntryCmd(base_addr);
  lld_LockRegBitsProgramCmd(base_addr,value);

#ifdef LLD_POLL_STATUS
	/* poll for completion */
    status_reg = lld_Poll(base_addr, 0);
    if( (status_reg & DEV_PROGRAM_MASK) == DEV_PROGRAM_MASK )
	    dev_status =  DEV_PROGRAM_ERROR;		/* program error */
	else
		dev_status = DEV_NOT_BUSY;

#elif defined LLD_STATUS_GET
  /* poll for completion */
  do
  {
      polling_counter--;
      /* no checking for interrupts */

      dev_status = lld_StatusGet((FLASHDATA *)base_addr, 0);
  } while ((dev_status == DEV_BUSY) && polling_counter);
#endif
  /* if not done, then we have an error */
  if (dev_status != DEV_NOT_BUSY)
  {
     /* Write Software RESET command */
     FLASH_WR(base_addr, 0, NOR_RESET_CMD);
     lld_LockRegExitCmd(base_addr);
     return (-1); /* return error */
  }

  lld_LockRegExitCmd(base_addr);
  return 0; /* successful */
}
#endif /* LLD_DEV_FLASH */
#endif

#ifdef LLD_LOCKREG_WR_OP_2
/******************************************************************************
*
* lld_SSRLockRegBitsProgramOp: Operation to program SSR Lock Register
*
* RETURNS: 0 Successful
*         -1 Failed
*
*/
int lld_SSRLockRegBitsProgramOp ( FLASHDATA *base_addr, ADDRESS offset, FLASHDATA value)
{
  FLASHDATA    status_reg;
  unsigned long  polling_counter = 0xFFFFFFFF;

  lld_SSRLockRegEntryCmd(base_addr, offset);
  lld_SSRLockRegBitsProgramCmd(base_addr, offset, value);

  /* poll for completion */
  status_reg = lld_Poll((FLASHDATA *)base_addr, offset);

  lld_SSRLockRegExitCmd(base_addr);		/* exit command */

  if( (status_reg & DEV_PROGRAM_MASK) == DEV_PROGRAM_MASK )
	  return( DEV_PROGRAM_ERROR );		/* program error */

  return( 0 );			    /* program complete */

}
#endif /* LLD_LOCKREG_WR_OP */


#ifdef LLD_SECTOR_LOCK_CMD
/******************************************************************************
*
* lld_SectorLockCmd: Lock all sectors in device command
*
* RETURNS: None
*
*/
void lld_SectorLockCmd (
FLASHDATA *base_addr,
ADDRESS offset
)
{
	// Unlock sequence command
	FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR1, NOR_SECTOR_LOCK_CMD);
	FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR2, NOR_SECTOR_LOCK_CMD);
    // Lock command
	FLASH_WR(base_addr, (offset & NOR_SECTOR_LOCK_ADR6), NOR_SECTOR_LOCK_CMD);
}
#endif /* LLD_SECTOR_LOCK_CMD */

#ifdef LLD_SECTOR_UNLOCK_CMD
/******************************************************************************
*
* lld_SectorUnlockCmd: Unlock a sector command
*
* RETURNS: None
*
*/
void lld_SectorUnlockCmd (
FLASHDATA *base_addr,
ADDRESS offset
)
{
	// unlock command
	FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR1, NOR_SECTOR_LOCK_CMD);
	FLASH_WR(base_addr, (offset & SA_OFFSET_MASK) + LLD_UNLOCK_ADDR2, NOR_SECTOR_LOCK_CMD);

	FLASH_WR(base_addr, (offset | NOR_SECTOR_UNLOCK_ADR6), NOR_SECTOR_LOCK_CMD);
}
#endif /* LLD_SECTOR_UNLOCK_CMD */

#ifdef LLD_LOCK_RANGE_CMD
/******************************************************************************
*
* lld_SectorLockRangeCmd: lock a range of sectors command
*
* RETURNS: 0 - Successful
*		   -1 - Failed
*
*/
int lld_SectorLockRangeCmd (
FLASHDATA *base_addr,
ADDRESS StartSec,			/* first sector in range */
ADDRESS StopSec             /* last sector in range */
)
{

	// Stop sector address must be greater than the start sector
	if( StartSec > StopSec )
		return(-1);

	// unlock sequence command
	FLASH_WR(base_addr, LLD_UNLOCK_ADDR1, NOR_SECTOR_LOCK_CMD);
	FLASH_WR(base_addr, LLD_UNLOCK_ADDR2, NOR_SECTOR_LOCK_CMD);

	// Load the start sector address with A6 set to zero
	FLASH_WR(base_addr, (StartSec & NOR_SECTOR_LOCK_ADR6), NOR_LOAD_SECTOR_ADR);
	// Load the stop sector address with A6 set to zero
	FLASH_WR(base_addr, (StopSec  & NOR_SECTOR_LOCK_ADR6), NOR_LOAD_SECTOR_ADR);

	return (0);
}
#endif /* LLD_LOCK_RANGE_CMD */
