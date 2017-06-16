#ifndef LIBS_DRIVERS_MSC_INCLUDE_MSC_MSC_HPP_
#define LIBS_DRIVERS_MSC_INCLUDE_MSC_MSC_HPP_

#include <cstdint>
#include <gsl/span>
#include "utils.h"

namespace drivers
{
    namespace msc
    {
        struct IMCUFlash
        {
            virtual void Erase(std::size_t sectorOffset) = 0;
            virtual void Program(std::size_t sectorOffset, gsl::span<std::uint8_t> buffer) = 0;
            virtual std::uint8_t const* Begin() const = 0;

            static constexpr std::size_t SectorSize = 4_KB;
        };

        class MCUMemoryController : public IMCUFlash
        {
          public:
            virtual void Erase(std::size_t sectorOffset) override;
            virtual void Program(std::size_t sectorOffset, gsl::span<std::uint8_t> buffer) override;

            virtual std::uint8_t const* Begin() const override;
        };
    }
}

#endif /* LIBS_DRIVERS_MSC_INCLUDE_MSC_MSC_HPP_ */
