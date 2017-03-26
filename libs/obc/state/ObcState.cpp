#include "ObcState.hpp"
#include <cstdint>
#include <gsl/span>
#include "IStorageAccess.hpp"
#include "base/reader.h"
#include "base/writer.h"
#include "logger/logger.h"
#include "state/PersistentState.hpp"

namespace obc
{
    static constexpr std::uint32_t Signature = 0x55aa77ee;

    static constexpr std::uint32_t PersistenStateImageSize = state::PersistentState::Size() + sizeof(Signature);

    bool WritePersistentState(const state::PersistentState& stateObject, std::uint32_t baseAddress, IStorageAccess& storage)
    {
        std::uint8_t array[PersistenStateImageSize];
        Writer writer(gsl::make_span(array));
        writer.WriteDoubleWordLE(Signature);
        stateObject.Write(writer);
        if (!writer.Status())
        {
            LOG(LOG_LEVEL_ERROR, "Unable to generate persistent state image.");
            return false;
        }

        storage.Write(baseAddress, writer.Capture());
        LOG(LOG_LEVEL_INFO, "Persistent state updated. ");
        return true;
    }

    bool ReadPersistentState(state::PersistentState& stateObject, std::uint32_t baseAddress, IStorageAccess& storage)
    {
        std::uint8_t array[PersistenStateImageSize];
        storage.Read(baseAddress, gsl::make_span(array));
        Reader reader(gsl::make_span(array));
        const auto signature = reader.ReadDoubleWordLE();
        if (                          //
            signature != Signature || //
            !reader.Status()          //
            )
        {
            LOG(LOG_LEVEL_ERROR, "Unable to parse persistent state image.");
            stateObject = state::PersistentState();
            return false;
        }

        stateObject.Read(reader);
        return reader.Status();
    }
}
