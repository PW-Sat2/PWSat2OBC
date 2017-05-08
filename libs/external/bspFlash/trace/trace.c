/* trace.c - Source Code for Spansion's LLD Virtual Logic Analyzer trace support */
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

#ifdef __SLLD
  #include "slld.h"
#elif __OLLD
  #include "olld.h"
#elif __D2LLD
  #include "d2lld.h"
  #include "d2lld_hal.h"
#elif __OPLLD
  #include "basic_lld.h"
#else
  #include "lld.h"
#endif
#include "trace.h"

unsigned int trace_sync = 0;
extern unsigned long os_GetCurrentTicks();

#define STORE_TIMESTAMP     trace[cycle].timestamp = os_GetCurrentTicks()


#define LLD_DEV_FLASH
//#define LLD_DEV_SIM


CYCLE trace[MAX_CYCLES];
int cycle = -1;
int trace_wrapped = FALSE;

#ifdef __SLLD
void StoreTraceCycle(OPERATION op, FLASHDATA command, ADDRESS sys_addr, FLASHDATA data, int Number_Of_Bytes)
{
  cycle++;
  if (cycle >= MAX_CYCLES) 
  {
    cycle = 0;
    trace_wrapped = TRUE;
  }

  trace[cycle].trace_sync      = trace_sync++;
  trace[cycle].op              = op;
  trace[cycle].command         = command;
  trace[cycle].sys_addr        = sys_addr;
  trace[cycle].data            = data;
  trace[cycle].Number_Of_Bytes = Number_Of_Bytes;
}

SLLD_STATUS FlashWrite(BYTE command, ADDRESS sys_addr, FLASHDATA *data_buffer, int Number_Of_Written_Bytes)
{
  SLLD_STATUS status;

  status = FLASH_WRITE(command, sys_addr, data_buffer, Number_Of_Written_Bytes);
  if (data_buffer == BUFFER_NOT_USED)
      StoreTraceCycle(TR_OP_WRITE, command, sys_addr, (FLASHDATA)0xFFFFFFFF, Number_Of_Written_Bytes);
  else
      StoreTraceCycle(TR_OP_WRITE, command, sys_addr, *data_buffer, Number_Of_Written_Bytes);
  
  #ifdef PRINT_TIMESTAMP
     STORE_TIMESTAMP;
  #endif
  
  return(status);
}

SLLD_STATUS FlashRead(BYTE command, ADDRESS sys_addr, FLASHDATA *data_buffer, int Number_Of_Read_Bytes)
{
  SLLD_STATUS status;

  status = FLASH_READ(command, sys_addr, data_buffer, Number_Of_Read_Bytes);
  if (data_buffer == BUFFER_NOT_USED)
      StoreTraceCycle(TR_OP_READ, command, sys_addr, (FLASHDATA)0xFFFFFFFF, Number_Of_Read_Bytes);
  else
      StoreTraceCycle(TR_OP_READ, command, sys_addr, *data_buffer, Number_Of_Read_Bytes);

  #ifdef PRINT_TIMESTAMP
     STORE_TIMESTAMP;
  #endif

  return(status);
}
#elif !(defined __D2LLD)
void StoreTraceCycle(OPERATION op, FLASHDATA * base, ADDRESS offset, ADDRESS addr, FLASHDATA data)
{
  cycle++;
  if (cycle >= MAX_CYCLES) 
  {
    cycle = 0;
    trace_wrapped = TRUE;
  }
  trace[cycle].trace_sync = trace_sync++;
  trace[cycle].op         = op;
  trace[cycle].base       = base;
  trace[cycle].offset     = offset;
  trace[cycle].addr       = addr;
  trace[cycle].data       = data;

}

#ifdef LLD_DEV_FLASH
void FlashWrite(FLASHDATA * addr, ADDRESS offset, FLASHDATA data)
{
#ifdef EXTEND_ADDR
  FlashWrite_Extend(addr, offset, data);
  StoreTraceCycle(TR_OP_WRITE, addr, offset, (ADDRESS)((volatile FLASHDATA*)(addr) + (offset)), data);
#else
  FLASH_OFFSET((addr),(offset)) = (data);
  StoreTraceCycle(TR_OP_WRITE, addr, offset, (ADDRESS)((volatile FLASHDATA*)(addr) + (offset)), data);
#endif
  
  #ifdef PRINT_TIMESTAMP
    STORE_TIMESTAMP;
  #endif
}
#endif  //LLD_DEV_FLASH

#ifdef LLD_DEV_FLASH
FLASHDATA FlashRead(FLASHDATA * addr, ADDRESS offset)
{
  FLASHDATA data = 0;

  #ifdef EXTEND_ADDR
    data = FlashRead_Extend(addr, offset);
    StoreTraceCycle(TR_OP_READ, addr, offset, (ADDRESS)((volatile FLASHDATA*)(addr) + (offset)), data);
  #else
    data = FLASH_OFFSET((addr),(offset));
    StoreTraceCycle(TR_OP_READ, addr, offset, (ADDRESS)((volatile FLASHDATA*)(addr) + (offset)), data);
  #endif

  #ifdef PRINT_TIMESTAMP
    STORE_TIMESTAMP;
  #endif

  return(data);
}
#endif // LLD_DEV_FLASH

#else //_D2LLD

void StoreTraceCycle(OPERATION op, FLASHDATA * base, ADDRESS offset, FLASHDATA * src, BYTECOUNT size)
{
  int i = size;

  while(i > 0)
  {
    cycle++;
    if (cycle >= MAX_CYCLES) 
    {
      cycle = 0;
      trace_wrapped = TRUE;
    }
    trace[cycle].trace_sync = trace_sync++;
    trace[cycle].op         = op;
    trace[cycle].base       = base;
    trace[cycle].offset     = offset;
    trace[cycle].src       = *src;
    trace[cycle].size       = LLD_BYTES_PER_OP;

    i -= LLD_BYTES_PER_OP;
    offset += LLD_BYTES_PER_OP;
    src++ ;
  }
}

void FlashWrite(FLASHDATA * addr, ADDRESS offset, FLASHDATA * src, BYTECOUNT size)
{
  d2lld_HalWrite(addr, offset, src, size);
  StoreTraceCycle(TR_OP_WRITE, addr, offset, src, size);

#ifdef PRINT_TIMESTAMP
  STORE_TIMESTAMP;
#endif

}

void FlashRead(FLASHDATA * addr, ADDRESS offset, FLASHDATA * buf, BYTECOUNT size)
{
  d2lld_HalRead(addr, offset, buf, size);
  StoreTraceCycle(TR_OP_READ, addr, offset, buf, size);

#ifdef PRINT_TIMESTAMP
   STORE_TIMESTAMP;
#endif
}

#endif // _D2LLD

#ifdef TRACE_NAND_SIGNAL
void FlashRecordSetSignal(LLD_UINT32 sig_no, LLD_UINT32 data)
{
  StoreTraceCycle(OP_SET_SIGNAL, 0, 0, (ADDRESS)sig_no, (FLASHDATA)data);
  
  #ifdef PRINT_TIMESTAMP
    STORE_TIMESTAMP;
  #endif
}

void FlashRecordGetSignal(LLD_UINT32 sig_no, LLD_UINT32 data)
{
  StoreTraceCycle(OP_GET_SIGNAL, 0, 0, (ADDRESS)sig_no, (FLASHDATA)data);
  
  #ifdef PRINT_TIMESTAMP
     STORE_TIMESTAMP;
  #endif
}
#endif  /* TRACE_NAND_SIGNAL */

