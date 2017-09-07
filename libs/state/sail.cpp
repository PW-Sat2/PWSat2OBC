#include "sail/SailState.hpp"

namespace state
{
    SailState::SailState(SailOpeningState currentState, bool deploymentDisabled)
        : _currentState(currentState), isDeploymentDisabled(deploymentDisabled)
    {
    }

    SailState::SailState() : _currentState(SailOpeningState::Waiting), isDeploymentDisabled(false)
    {
    }

    void SailState::Read(Reader& reader)
    {
        this->_currentState = static_cast<SailOpeningState>(reader.ReadByte());
        this->isDeploymentDisabled = reader.ReadByte();
    }

    void SailState::Write(Writer& writer) const
    {
        writer.WriteByte(num(this->_currentState));
        writer.WriteByte(this->isDeploymentDisabled);
    }
}
