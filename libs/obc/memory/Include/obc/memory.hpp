#ifndef LIBS_OBC_MEMORY_INCLUDE_OBC_MEMORY_HPP_
#define LIBS_OBC_MEMORY_INCLUDE_OBC_MEMORY_HPP_

#include <array>
#include "system.h"

namespace obc
{
    /**
     * @defgroup obc_memory SRAM memory handling
     *
     * This module contains mechanisms needed for SRAM modules control (disable on latch-up and recovery)
     *
     * @{
     */

    /**
     * @brief Available memory module
     */
    enum class MemoryModule
    {
        SRAM1 = 0, //!< SRAM1
        SRAM2 = 1  //!< SRAM2
    };

    /**
     * @brief Memory module manager
     *
     * Initially all modules are marked as latched-up. This ensures proper recovery even if latch-up happen during starup and was not
     * detected.
     */
    class OBCMemory
    {
      public:
        /**
         * @brief Ctor
         */
        OBCMemory();

        /**
         * @brief Handles latch-up of memory module
         * @param memory Memory module on which latch-up occured
         * @remark Called by ACMP IRQ Handler
         */
        void HandleLatchup(MemoryModule memory);

        /**
         * @brief Performs recovery procedure on latched-up modules
         */
        void Recover();

        /**
         * @brief Checks if module is marked as latched-up
         * @param memory Memory module to check
         * @return true if module is marked as latched-up
         */
        inline bool IsLatched(MemoryModule memory) const;

      private:
        /** @brief Memory modules latched-up flag */
        std::array<bool, 2> _sramLatched;
        /** @brief Recovery retries */
        static constexpr std::uint8_t RecoveryRetries = 3;
    };

    bool OBCMemory::IsLatched(MemoryModule memory) const
    {
        return this->_sramLatched[num(memory)];
    }

    /** @} */
}

#endif /* LIBS_OBC_MEMORY_INCLUDE_OBC_MEMORY_HPP_ */
