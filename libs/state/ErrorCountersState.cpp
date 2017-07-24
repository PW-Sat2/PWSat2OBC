#include "fdir/ErrorCountersState.hpp"
#include "base/reader.h"
#include "base/writer.h"

namespace state
{
    ErrorCountersConfigState::ErrorCountersConfigState()
    {
        // Limit: 128, Increment: 5, Decrement: 2
        this->_config.fill((128) | (5 << 8) | (2 << 16));
    }

    ErrorCountersConfigState::ErrorCountersConfigState(const ConfigBuffer& currentConfig) : _config(currentConfig)
    {
    }

    void ErrorCountersConfigState::Read(Reader& reader)
    {
        for (std::size_t i = 0; i < error_counter::ErrorCounting::MaxDevices; i++)
        {
            auto v = reader.ReadDoubleWordLE();
            this->_config[i] = v;
        }
    }

    void ErrorCountersConfigState::Write(Writer& writer) const
    {
        for (std::size_t i = 0; i < error_counter::ErrorCounting::MaxDevices; i++)
        {
            writer.WriteDoubleWordLE(this->_config[i]);
        }
    }
}
