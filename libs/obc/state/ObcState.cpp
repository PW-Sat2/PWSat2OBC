#include "ObcState.hpp"
#include <cstdint>
#include <gsl/span>
#include "IStorageAccess.hpp"
#include "base/reader.h"
#include "base/writer.h"
#include "logger/logger.h"
#include "state/struct.h"

namespace obc
{
    static constexpr std::uint32_t Signature = 0x55aa77ee;

    static constexpr std::uint32_t TotalImageSize = state::SystemPersistentState::Size() + 2 * sizeof(Signature);

    bool WritePersistentState(const state::SystemPersistentState& stateObject, std::uint32_t baseAddress, IStorageAccess& storage)
    {
        std::uint8_t array[TotalImageSize];
        Writer writer(gsl::make_span(array));
        writer.WriteDoubleWordLE(Signature);
        stateObject.Capture(writer);
        writer.WriteDoubleWordLE(Signature);
        if (!writer.Status())
        {
            LOG(LOG_LEVEL_ERROR, "Unable to generate persistent state image.");
            return false;
        }

        storage.Write(baseAddress, writer.Capture());
        LOG(LOG_LEVEL_INFO, "Persistent state updated. ");
        return true;
    }

    bool ReadPersistentState(state::SystemPersistentState& stateObject, std::uint32_t baseAddress, IStorageAccess& storage)
    {
        alignas(4) std::uint8_t array[TotalImageSize];
        storage.Read(baseAddress, gsl::make_span(array));
        Reader reader(gsl::make_span(array));
        const auto header = reader.ReadDoubleWordLE();
        if (                       //
            header != Signature || //
            !reader.Status()       //
            )
        {
            LOG(LOG_LEVEL_ERROR, "Unable to parse persistent state image.");
            stateObject = state::SystemPersistentState();
            return false;
        }

        stateObject.Read(reader);
        const auto footer = reader.ReadDoubleWordLE();
        return footer == Signature && reader.Status();
    }
}
