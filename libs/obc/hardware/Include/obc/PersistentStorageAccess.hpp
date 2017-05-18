#ifndef LIBS_OBC_STATE_PERSISTENT_STORAGE_ACCESS_HPP
#define LIBS_OBC_STATE_PERSISTENT_STORAGE_ACCESS_HPP

#pragma once

#include "fm25w/fm25w.hpp"
#include "obc/IStorageAccess.hpp"
#include "spi/spi.h"

/**
 * @addtogroup StateDef
 * @{
 */
namespace obc
{
    /**
     * @brief This type bridges the persistent state serialization routines and the fram memory that should be used
     * as its intended long term storage.
     */
    class PersistentStorageAccess final : public IStorageAccess
    {
      public:
        /**
         * @brief ctor.
         * @param[in] spi Reference to spi bus controller that should be used to access the memory.
         */
        PersistentStorageAccess(std::array<drivers::spi::ISPIInterface*, 3> spis);

        virtual void Read(std::uint32_t address, gsl::span<std::uint8_t> span) final override;

        virtual void Write(std::uint32_t address, gsl::span<const std::uint8_t> span) final override;

      private:
        /** @brief Fram memory controllers. */
        devices::fm25w::FM25WDriver _fm25wDrivers[3];

        /** @brief Redundant Fram memory driver. */
        devices::fm25w::RedundantFM25WDriver _driver;
    };
}

/** @} */
#endif
