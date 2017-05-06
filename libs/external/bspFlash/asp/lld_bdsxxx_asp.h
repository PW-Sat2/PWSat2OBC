/* lld_bdsxxx_asp.h - Source Code for Spansion's Low Level Driver */
/* v5.4 */
/**************************************************************************
* Copyright (C)2011 Spansion Inc. All Rights Reserved. 
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
DEVSTATUS lld_PasswordProtectionProgramOp
(
FLASHDATA * base_addr,
FLASHDATA pwd0,
FLASHDATA pwd1,
FLASHDATA pwd2,
FLASHDATA pwd3
);

void lld_PasswordProtectionPasswordVerify
( 
FLASHDATA * base_addr,
FLASHDATA *pd0,
FLASHDATA *pd1,
FLASHDATA *pd2,
FLASHDATA *pd3
);

void lld_PasswordProtectionPasswordUnlock
(
FLASHDATA * base_addr,
FLASHDATA pd0,
FLASHDATA pd1,
FLASHDATA pd2,
FLASHDATA pd3
);

FLASHDATA PPB_Status
(
FLASHDATA * base_addr,
ADDRESS   offset
);

FLASHDATA PPB_Program
(
FLASHDATA * base_addr,
ADDRESS   offset
);

FLASHDATA PPB_AllErase
(
FLASHDATA * base_addr,
ADDRESS   offset
);

void PPB_LockBitSet
(
FLASHDATA * base_addr
);

FLASHDATA PPB_LockBitStatus
(
FLASHDATA * base_addr
);

void DYB_Write
(
FLASHDATA * base_addr,
ADDRESS   offset
);

void DYB_Erase
(
FLASHDATA * base_addr,
ADDRESS   offset
);

FLASHDATA DYB_Status
(
FLASHDATA * base_addr,
ADDRESS   offset
);

FLASHDATA lld_PasswordProtectionModeLockingBitProgram
(
FLASHDATA * base_addr
);

FLASHDATA lld_PersistentProtectionModeLockingBitProgram
(
FLASHDATA * base_addr
);

FLASHDATA lld_PasswordProtectionModeLockingBitRead
(
FLASHDATA * base_addr
);

FLASHDATA lld_PersistentProtectionModeLockingBitRead
(
FLASHDATA * base_addr
);



