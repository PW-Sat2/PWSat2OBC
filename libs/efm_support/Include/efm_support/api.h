#ifndef LIBS_EFM_SUPPORT_INCLUDE_EFM_SUPPORT_API_H_
#define LIBS_EFM_SUPPORT_INCLUDE_EFM_SUPPORT_API_H_

#include <em_cmu.h>
#include <em_usart.h>
#include "dmadrv.h"

namespace efm
{
    /**
     * @ingroup efm_support
     * @{
     */

    namespace cmu
    {
        /***************************************************************************/ /**
          * @brief
          *   Enable/disable a clock.
          *
          * @details
          *   In general, module clocking is disabled after a reset. If a module
          *   clock is disabled, the registers of that module are not accessible and
          *   reading from such registers may return undefined values. Writing to
          *   registers of clock disabled modules have no effect. One should normally
          *   avoid accessing module registers of a module with a disabled clock.
          *
          * @note
          *   If enabling/disabling a LF clock, synchronization into the low frequency
          *   domain is required. If the same register is modified before a previous
          *   update has completed, this function will stall until the previous
          *   synchronization has completed. Please refer to CMU_FreezeEnable() for
          *   a suggestion on how to reduce stalling time in some use cases.
          *
          * @param[in] clock
          *   The clock to enable/disable. Notice that not all defined clock
          *   points have separate enable/disable control, please refer to CMU overview
          *   in reference manual.
          *
          * @param[in] enable
          *   @li true - enable specified clock.
          *   @li false - disable specified clock.
          *
          * @ingroup efm_support
          ******************************************************************************/
        void ClockEnable(CMU_Clock_TypeDef clock, bool enable);
    }

    namespace usart
    {
        /***************************************************************************/ /**
          * @brief
          *   Init USART for synchronous mode.
          *
          * @details
          *   This function will configure basic settings in order to operate in
          *   synchronous mode.
          *
          *   Special control setup not covered by this function must be done after
          *   using this function by direct modification of the CTRL register.
          *
          *   Notice that pins used by the USART module must be properly configured
          *   by the user explicitly, in order for the USART to work as intended.
          *   (When configuring pins, one should remember to consider the sequence of
          *   configuration, in order to avoid unintended pulses/glitches on output
          *   pins.)
          *
          * @param[in] usart
          *   Pointer to USART peripheral register block. (UART does not support this
          *   mode.)
          *
          * @param[in] init
          *   Pointer to initialization structure used to configure basic async setup.
          *
          * @ingroup efm_support
          ******************************************************************************/
        void InitSync(USART_TypeDef* usart, const USART_InitSync_TypeDef* init);

        /***************************************************************************/ /**
          * @brief
          *   Enable/disable USART/UART receiver and/or transmitter.
          *
          * @details
          *   Notice that this function does not do any configuration. Enabling should
          *   normally be done after initialization is done (if not enabled as part
          *   of init).
          *
          * @param[in] usart
          *   Pointer to USART/UART peripheral register block.
          *
          * @param[in] enable
          *   Select status for receiver/transmitter.
          *
          * @ingroup efm_support
          ******************************************************************************/
        void Enable(USART_TypeDef* usart, USART_Enable_TypeDef enable);

        /***************************************************************************/ /**
          * @brief
          *   Clear one or more pending USART interrupts.
          *
          * @param[in] usart
          *   Pointer to USART/UART peripheral register block.
          *
          * @param[in] flags
          *   Pending USART/UART interrupt source(s) to clear. Use one or more valid
          *   interrupt flags for the USART module (USART_IF_nnn) OR'ed together.
          *
          * @ingroup efm_support
          ******************************************************************************/
        void IntClear(USART_TypeDef* usart, uint32_t flags);

        /***************************************************************************/ /**
          * @brief
          *   Get pending USART interrupt flags.
          *
          * @note
          *   The event bits are not cleared by the use of this function.
          *
          * @param[in] usart
          *   Pointer to USART/UART peripheral register block.
          *
          * @return
          *   USART/UART interrupt source(s) pending. Returns one or more valid
          *   interrupt flags for the USART module (USART_IF_nnn) OR'ed together.
          *
          * @ingroup efm_support
          ******************************************************************************/
        uint32_t IntGet(USART_TypeDef* usart);

        /**
         * @brief Amends USART route register by ORing additional flags
         * @param usart USART peripheral
         * @param flags Flag that should be OR with route register
         *
         * @ingroup efm_support
         */
        void AmendRoute(USART_TypeDef* usart, uint32_t flags);

        /**
         * @brief Writes value to USART command register
         * @param usart USART peripheral
         * @param cmd Command to write
         *
         * @ingroup efm_support
         */
        void Command(USART_TypeDef* usart, uint32_t cmd);
    }

    namespace dma
    {
        /***************************************************************************/ /**
          * @brief
          *  Allocate (reserve) a DMA channel.
          *
          * @param[out] channelId
          *  The channel Id assigned by DMADRV.
          *
          * @param[in] capabilities
          *  Not used.
          *
          * @return
          *  ECODE_EMDRV_DMADRV_OK on success. On failure an appropriate
          *  DMADRV Ecode_t is returned.
          *
          * @ingroup efm_support
          ******************************************************************************/
        Ecode_t AllocateChannel(unsigned int* channelId, void* capabilities);

        /***************************************************************************/ /**
          * @brief
          *  Start a peripheral to memory DMA transfer.
          *
          * @param[in] channelId
          *  Not used
          *
          * @param[in] peripheralSignal
          *  Selects which peripheral/peripheralsignal to use.
          *
          * @param[in] dst
          *  Destination memory address.
          *
          * @param[in] src
          *  Source memory (peripheral register) address.
          *
          * @param[in] dstInc
          *  Set to true to enable destination address increment (increment is according
          *  to @a size parameter).
          *
          * @param[in] len
          *  Number if items (of @a size size) to transfer.
          *
          * @param[in] size
          *  Item size, byte, halfword or word.
          *
          * @param[in] callback
          *  Function to call on dma completion, use NULL if not needed.
          *
          * @param[in] cbUserParam
          *  Optional user parameter to feed to the callback function. Use NULL if
          *  not needed.
          *
          * @return
          *   ECODE_EMDRV_DMADRV_OK on success. On failure an appropriate
          *   DMADRV Ecode_t is returned.
          *
          * @ingroup efm_support
          ******************************************************************************/
        Ecode_t PeripheralMemory(unsigned int channelId,
            DMADRV_PeripheralSignal_t peripheralSignal,
            void* dst,
            void* src,
            bool dstInc,
            int len,
            DMADRV_DataSize_t size,
            DMADRV_Callback_t callback,
            void* cbUserParam);

        /***************************************************************************/ /**
          * @brief
          *  Start a memory to peripheral DMA transfer.
          *
          * @param[in] channelId
          *  Not used
          *
          * @param[in] peripheralSignal
          *  Selects which peripheral/peripheralsignal to use.
          *
          * @param[in] dst
          *  Destination (peripheral register) memory address.
          *
          * @param[in] src
          *  Source memory address.
          *
          * @param[in] srcInc
          *  Set to true to enable source address increment (increment is according to
          *  @a size parameter).
          *
          * @param[in] len
          *  Number if items (of @a size size) to transfer.
          *
          * @param[in] size
          *  Item size, byte, halfword or word.
          *
          * @param[in] callback
          *  Function to call on dma completion, use NULL if not needed.
          *
          * @param[in] cbUserParam
          *  Optional user parameter to feed to the callback function. Use NULL if
          *  not needed.
          *
          * @return
          *   ECODE_EMDRV_DMADRV_OK on success. On failure an appropriate
          *   DMADRV Ecode_t is returned.
          *
          * @ingroup efm_support
          ******************************************************************************/
        Ecode_t MemoryPeripheral(unsigned int channelId,
            DMADRV_PeripheralSignal_t peripheralSignal,
            void* dst,
            void* src,
            bool srcInc,
            int len,
            DMADRV_DataSize_t size,
            DMADRV_Callback_t callback,
            void* cbUserParam);
    }

    /** @} */
}

#endif /* LIBS_EFM_SUPPORT_INCLUDE_EFM_SUPPORT_API_H_ */
