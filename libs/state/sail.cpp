#include "sail/SailState.hpp"

namespace state
{
    SailState::SailState(SailOpeningState currentState) : _currentState(currentState)
    {
    }

    SailState::SailState() : _currentState(SailOpeningState::Waiting)
    {
    }

    void SailState::Read(Reader& reader)
    {
        this->_currentState = static_cast<SailOpeningState>(reader.ReadByte());
    }

    void SailState::Write(Writer& writer) const
    {
        writer.WriteByte(num(this->_currentState));
    }
}
