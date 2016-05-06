#include <em_system.h>
#include <core_cm3.h>
#include <em_cmu.h>
#include <em_dbg.h>

/** \brief  ITM Send Character to specific channel

    The function transmits a character via the ITM channel 0, and
    \li Just returns when no debugger is connected that has booked the output.
    \li Is blocking when a debugger is connected, but the previous character sent has not been transmitted.

    \param [in]     ch  Character to transmit.

    \returns            Character to transmit.
 */
uint32_t ITM_SendCharToChannel (uint32_t ch, uint8_t channel)
{
  if ((ITM->TCR & ITM_TCR_ITMENA_Msk)                  &&      /* ITM enabled */
      (ITM->TER & (1UL << channel)        )                    )     /* ITM Port #0 enabled */
  {
    while (ITM->PORT[channel].u32 == 0);
    ITM->PORT[channel].u8 = (uint8_t) ch;
  }
  return (ch);
}

