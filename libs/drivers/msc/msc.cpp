#include "msc.hpp"
#include <em_msc.h>

namespace drivers
{
    namespace msc
    {
        void MCUMemoryController::Erase(std::size_t sectorOffset)
        {
            MSC_Init();

            MSC_ErasePage(reinterpret_cast<uint32_t*>(sectorOffset));

            MSC_Deinit();
        }

        void MCUMemoryController::Program(std::size_t sectorOffset, gsl::span<std::uint8_t> buffer)
        {
            MSC_Init();

            auto writeAt = reinterpret_cast<uint32_t*>(sectorOffset);

            MSC_WriteWord(writeAt, buffer.data(), buffer.size());

            MSC_Deinit();
        }

        std::uint8_t const* MCUMemoryController::Begin() const
        {
            return reinterpret_cast<std::uint8_t const*>(0);
        }
    }
}
