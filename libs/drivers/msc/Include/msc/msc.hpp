#ifndef LIBS_DRIVERS_MSC_INCLUDE_MSC_MSC_HPP_
#define LIBS_DRIVERS_MSC_INCLUDE_MSC_MSC_HPP_

#include <cstdint>
#include <gsl/span>
#include "utils.h"

namespace drivers
{
    namespace msc
    {
        class MCUMemoryController
        {
          public:
            void Erase(std::size_t sectorOffset);
            void Program(std::size_t sectorOffset, gsl::span<std::uint8_t> buffer);

            static constexpr std::size_t SectorSize = 4_KB;
        };
    }
}

#endif /* LIBS_DRIVERS_MSC_INCLUDE_MSC_MSC_HPP_ */
