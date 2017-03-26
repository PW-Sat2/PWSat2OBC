#ifndef LIBS_OBC_STATE_PERSISTENT_STORAGE_ACCESS_HPP
#define LIBS_OBC_STATE_PERSISTENT_STORAGE_ACCESS_HPP

#pragma once

#include "IStorageAccess.hpp"
#include "fm25w/fm25w.hpp"
#include "spi/spi.h"

namespace obc
{
    class PersistentStorageAccess final : public IStorageAccess
    {
      public:
        PersistentStorageAccess(drivers::spi::ISPIInterface& spi);

        virtual void Read(std::uint32_t address, gsl::span<std::uint8_t> span) final override;

        virtual void Write(std::uint32_t address, gsl::span<const std::uint8_t> span) final override;

      private:
        devices::fm25w::FM25WDriver fram;
    };
}

#endif
