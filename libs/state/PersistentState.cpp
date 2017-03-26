#include "PersistentState.hpp"

namespace state
{
    void PersistentState::Read(Reader& reader)
    {
        this->antennaConfiguration.Read(reader);
    }

    void PersistentState::Write(Writer& writer) const
    {
        this->antennaConfiguration.Write(writer);
    }
}
