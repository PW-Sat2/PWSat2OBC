/* lld_wsxxx_asp.h - Source Code for Spansion's Low Level Driver */
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

#ifndef LLD_WSXXX_ASP_H
#define LLD_WSXXX_ASP_H

#define PPB_PROTECTED                           (0*LLD_DEV_MULTIPLIER)
#define PPB_UNPROTECTED                         (1*LLD_DEV_MULTIPLIER)

extern void lld_LockRegEntryCmd
(
  FLASHDATA *   base_addr
);

extern void lld_LockRegBitsProgramCmd
(
  FLASHDATA *   base_addr,
  FLASHDATA value
);

extern FLASHDATA lld_LockRegBitsReadCmd
(
  FLASHDATA *   base_addr
);

extern void lld_LockRegExitCmd
(
  FLASHDATA *   base_addr
);


extern void lld_PasswordProtectionEntryCmd
(
  FLASHDATA *   base_addr
);

extern void lld_PasswordProtectionProgramCmd
(
  FLASHDATA *   base_addr,
  ADDRESS   offset,
  FLASHDATA pwd
);

extern void lld_PasswordProtectionReadCmd
(
  FLASHDATA *   base_addr,
  FLASHDATA *pwd0,
  FLASHDATA *pwd1,
  FLASHDATA *pwd2,
  FLASHDATA *pwd3
);

extern void lld_PasswordProtectionUnlockCmd
(
  FLASHDATA *   base_addr,
  FLASHDATA pwd0,
  FLASHDATA pwd1,
  FLASHDATA pwd2,
  FLASHDATA pwd3
);

extern void lld_PasswordProtectionExitCmd
(
  FLASHDATA *   base_addr
);

extern void lld_PpbEntryCmd
(
  FLASHDATA * base_addr,
  ADDRESS     offset
);

extern void lld_PpbProgramCmd
(
  FLASHDATA *   base_addr,
  ADDRESS   offset
);

extern void lld_PpbAllEraseCmd
(
  FLASHDATA *   base_addr
);

extern FLASHDATA lld_PpbStatusReadCmd
(
  FLASHDATA *   base_addr,
  ADDRESS   offset
);

extern void lld_PpbExitCmd
(
  FLASHDATA *   base_addr
);

extern void lld_PpbLockBitEntryCmd
(
  FLASHDATA *   base_addr
);

extern void lld_PpbLockBitSetCmd
(
  FLASHDATA *   base_addr
);

extern FLASHDATA lld_PpbLockBitReadCmd
(
  FLASHDATA *   base_addr
);

extern void lld_PpbLockBitExitCmd
(
  FLASHDATA *   base_addr
);

extern void lld_DybEntryCmd
(
  FLASHDATA *   base_addr
);

extern void lld_DybSetCmd
(
  FLASHDATA *   base_addr,
  ADDRESS   offset
);

extern void lld_DybClrCmd
(
  FLASHDATA *   base_addr,
  ADDRESS   offset
);

extern FLASHDATA lld_DybReadCmd
(
  FLASHDATA *   base_addr,
  ADDRESS   offset
);

extern void lld_DybExitCmd
(
  FLASHDATA *   base_addr
);

extern FLASHDATA  lld_PpbLockBitReadOp ( FLASHDATA *base_addr);
extern int lld_PpbAllEraseOp ( FLASHDATA *   base_addr);
extern FLASHDATA lld_PpbStatusReadOp ( FLASHDATA *base_addr, ADDRESS offset);
extern int lld_PpbProgramOp ( FLASHDATA *base_addr, ADDRESS offset);
extern int lld_PpbLockBitSetOp ( FLASHDATA *   base_addr);
extern FLASHDATA lld_LockRegBitsReadOp ( FLASHDATA *base_addr);
extern int lld_LockRegBitsProgramOp ( FLASHDATA *base_addr, FLASHDATA value);

extern void lld_SectorLockCmd ( FLASHDATA *base_addr,ADDRESS offset );
extern void lld_SectorUnlockCmd ( FLASHDATA *base_addr, ADDRESS offset);
extern int lld_SectorLockRangeCmd ( FLASHDATA *base_addr, ADDRESS Startoffset, ADDRESS Endoffset);

#endif
