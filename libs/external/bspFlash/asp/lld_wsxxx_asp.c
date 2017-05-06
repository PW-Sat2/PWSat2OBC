/* lld_wsxxx.asp.c - Source Code for Spansion's Advance Sector Protection Driver */
/* v7.3.0 */
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
*/
#include "lld.h"
#include "lld_wsxxx_asp.h"

#define WSXXX_LOCK_REG_ENTRY      (0x40*LLD_DEV_MULTIPLIER)
#define WSXXX_PSWD_PROT_CMD_ENTRY (0x60*LLD_DEV_MULTIPLIER)
#define WSXXX_PSWD_UNLOCK_1       (0x25*LLD_DEV_MULTIPLIER)
#define WSXXX_PSWD_UNLOCK_2       (0x03*LLD_DEV_MULTIPLIER)
#define WSXXX_PSWD_UNLOCK_3       (0x29*LLD_DEV_MULTIPLIER)
#define WSXXX_PPB_ENTRY           (0xC0*LLD_DEV_MULTIPLIER)
#define WSXXX_PPB_ERASE_CONFIRM   (0x30*LLD_DEV_MULTIPLIER)
#define WSXXX_PPB_LOCK_ENTRY      (0x50*LLD_DEV_MULTIPLIER)
#define WSXXX_DYB_ENTRY           (0xE0*LLD_DEV_MULTIPLIER)
#define WSXXX_DYB_CLEAR           (0x01*LLD_DEV_MULTIPLIER)

/* WSxxx Commands */

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
  FLASH_WR(base_addr, 0x77, value);

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

	return(FLASH_RD(base_addr, 0x77));

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
FLASHDATA *pwd0,
FLASHDATA *pwd1,
FLASHDATA *pwd2,
FLASHDATA *pwd3
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
FLASHDATA pwd0,
FLASHDATA pwd1,
FLASHDATA pwd2,
FLASHDATA pwd3
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
ADDRESS       offset		
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



/****************************************************************************
*                                                                            *
* Section below are example codes to do operations using Ppb commands.       *
*                                                                            *
******************************************************************************/

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
  

    if (lld_PpbLockBitReadOp(base_addr) == PPB_PROTECTED)    /* if it is already locked */
    {
       return(-1);                            /* return an error cuz Lock Bit is locked */
    }


    lld_PpbEntryCmd(base_addr, 0);
    lld_PpbAllEraseCmd(base_addr);

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
        lld_ResetCmd((FLASHDATA *)base_addr);
        lld_PpbExitCmd(base_addr);
        return (-1); /* return error */
    }

    lld_PpbExitCmd(base_addr);		/* exit Ppb protection mode command */
    return 0; /* successful */
}

/******************************************************************************
* 
* lld_PpbStatusReadOp - Operation to read the Persistent Protection status register.
*
* RETURNS: FLASHDATA
*
*/

FLASHDATA lld_PpbStatusReadOp ( FLASHDATA *base_addr, ADDRESS offset)
{
   FLASHDATA  status;

   lld_PpbEntryCmd(base_addr,offset);
   status = (FLASH_RD(base_addr, offset));
   lld_PpbExitCmd(base_addr);
    
   return status;

}

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
  
  if (lld_PpbLockBitReadOp(base_addr) == PPB_PROTECTED)      /* if it is already locked */
  {
     return(-1);                              /* return an error cuz Lock Bit is locked */
  }


  lld_PpbEntryCmd(base_addr,offset);
  lld_PpbProgramCmd(base_addr, offset);

  /* poll for completion */
  do
  {
     polling_counter--;
     /* no checking for interrupts */
     /* poll Sector 0 for completion - workaround */
     dev_status = lld_StatusGet((FLASHDATA *)base_addr, 0);

  } while ((dev_status == DEV_BUSY) && polling_counter);

  /* if not done, then we have an error */
  if (dev_status != DEV_NOT_BUSY)
  {
     lld_ResetCmd((FLASHDATA *)base_addr);
     lld_PpbExitCmd(base_addr);
     return (-1); /* return error */
  }

  lld_PpbExitCmd(base_addr);
  return 0; /* successful */
}

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
  
    lld_PpbLockBitEntryCmd(base_addr);
    lld_PpbLockBitSetCmd(base_addr);

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
        lld_ResetCmd((FLASHDATA *)base_addr);
        lld_PpbLockBitExitCmd(base_addr);
        return (-1);  /* return error */ 
    }

    lld_PpbLockBitExitCmd(base_addr);
    return 0; /* successfull */
}

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

  lld_LockRegEntryCmd(base_addr);
  lld_LockRegBitsProgramCmd(base_addr,value);

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
     lld_ResetCmd((FLASHDATA *)base_addr);
     lld_LockRegExitCmd(base_addr);
     return (-1); /* return error */
  }

  lld_LockRegExitCmd(base_addr);
  return 0; /* successful */
}

