/* trace.h - Source Code for Spansion's LLD Virtual Logic Analyzer trace support */
/**************************************************************************
* Copyright 2009 Spansion LLC. All Rights Reserved. 
*
* This software is owned and published by: 
* Spansion LLC, 915 DeGuigne Drive, Sunnyvale, CA 94088 ("Spansion").
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
* Spansion MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE, 
* REGARDING THE SOFTWARE, ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED 
* USE, INCLUDING, WITHOUT LIMITATION, NO IMPLIED WARRANTY OF MERCHANTABILITY, 
* FITNESS FOR A  PARTICULAR PURPOSE OR USE, OR NONINFRINGEMENT.  Spansion WILL 
* HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT, NEGLIGENCE OR 
* OTHERWISE) FOR ANY DAMAGES ARISING FROM USE OR INABILITY TO USE THE SOFTWARE, 
* INCLUDING, WITHOUT LIMITATION, ANY DIRECT, INDIRECT, INCIDENTAL, 
* SPECIAL, OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA, SAVINGS OR PROFITS, 
* EVEN IF Spansion HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.  
*
* This software may be replicated in part or whole for the licensed use, 
* with the restriction that this Copyright notice must be included with 
* this software, whether used in part or whole, at all times.  
*/

#ifndef TRACE_INC_H
#define TRACE_INC_H

#ifdef __OLLD
#include "olld.h"
#elif __D2LLD
#include "d2lld.h"
#elif __OPLLD
#include "basic_lld.h"
#include "ext_lld.h"
#elif __SLLD
#include "slld.h"
#else
#include "lld.h"
#endif

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

// Turn Trace Time stamps on/off
// #define PRINT_TIMESTAMP

typedef LLD_UINT32  TIMESTAMP;

#ifdef TRACE_NAND_SIGNAL
typedef enum
{
  TR_OP_READ = 1,
  TR_OP_WRITE,
  OP_GET_SIGNAL,
  OP_SET_SIGNAL
}OPERATION;
#else	// original
typedef enum
{
  TR_OP_READ = 1,
  TR_OP_WRITE
}OPERATION;
#endif

#ifdef TRACE_NAND_SIGNAL
void StoreTraceCycle(OPERATION op, FLASHDATA * base, ADDRESS offset, ADDRESS addr, FLASHDATA data);
#endif

#ifdef __D2LLD
typedef struct
{
  unsigned int trace_sync;
  OPERATION op;
  FLASHDATA *   base;
  ADDRESS   offset;
  unsigned int src;
  unsigned int size;
#ifdef PRINT_TIMESTAMP
  TIMESTAMP timestamp;
#endif
}CYCLE;

#elif (defined __SLLD)
typedef struct
{
  unsigned int trace_sync;
  OPERATION    op;
  FLASHDATA    command;
  ADDRESS      sys_addr;
  FLASHDATA    data;
  int          Number_Of_Bytes;
#ifdef PRINT_TIMESTAMP
  TIMESTAMP    timestamp;
#endif
}CYCLE;

#else
typedef struct
{
  unsigned int trace_sync;
  OPERATION op;
  FLASHDATA *   base;
  ADDRESS   offset;
  ADDRESS   addr;
  FLASHDATA data;
#ifdef PRINT_TIMESTAMP
  TIMESTAMP timestamp;
#endif
}CYCLE;
#endif

#define MAX_CYCLES 512

#ifndef TRACE_C
extern CYCLE trace[MAX_CYCLES];
extern int cycle;
extern int trace_wrapped;
#endif

#ifdef __SLLD
SLLD_STATUS FlashRead(BYTE command, ADDRESS sys_addr, FLASHDATA *data_buffer, int Number_Of_Read_Bytes);
SLLD_STATUS FlashWrite(BYTE command, ADDRESS sys_addr, FLASHDATA *data_buffer, int Number_Of_Written_Bytes);
extern SLLD_STATUS FLASH_READ(BYTE command, ADDRESS sys_addr, FLASHDATA *data_buffer, int Number_Of_Read_Bytes);
extern SLLD_STATUS FLASH_WRITE(BYTE command, ADDRESS sys_addr, FLASHDATA *data_buffer, int Number_Of_Written_Bytes);
#elif (defined __D2LLD)
extern void FlashWrite(FLASHDATA * addr, ADDRESS offset, FLASHDATA * src, BYTECOUNT size);
extern void FlashRead(FLASHDATA * addr, ADDRESS offset, FLASHDATA * buf, BYTECOUNT size);
#else
extern void FlashWrite(FLASHDATA * addr, ADDRESS offset, FLASHDATA data);
extern FLASHDATA FlashRead(FLASHDATA * addr, ADDRESS offset);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TRACE_INC_H */
