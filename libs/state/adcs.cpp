#include "adcs/AdcsState.hpp"
#include "base/reader.h"
#include "base/writer.h"

namespace state
{
    AdcsState::AdcsState() : internalDetumblingDisabled(false)
    {
    }

    AdcsState::AdcsState(bool isInternalDetumblingDisabled) : internalDetumblingDisabled(isInternalDetumblingDisabled)
    {
    }

    void AdcsState::Read(Reader& reader)
    {
        this->internalDetumblingDisabled = reader.ReadByte() != 0;
    }

    void AdcsState::Write(Writer& writer) const
    {
        writer.WriteByte(this->internalDetumblingDisabled);
    }
}
