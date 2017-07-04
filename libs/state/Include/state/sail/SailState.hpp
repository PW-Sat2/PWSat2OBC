#ifndef LIBS_STATE_INCLUDE_STATE_SAIL_SAILSTATE_HPP_
#define LIBS_STATE_INCLUDE_STATE_SAIL_SAILSTATE_HPP_

#include "base/reader.h"
#include "base/writer.h"

namespace state
{
    enum class SailOpeningState : std::uint8_t
    {
        Waiting,
        Opening,
        OpenningStopped
    };

    class SailState
    {
      public:
        SailState() : _currentState(SailOpeningState::Waiting)
        {
        }

        SailState(SailOpeningState currentState) : _currentState(currentState)
        {
        }

        SailOpeningState CurrentState() const
        {
            return this->_currentState;
        }

        static constexpr std::size_t Size()
        {
            return 1;
        }

        void Read(Reader& reader)
        {
            this->_currentState = static_cast<SailOpeningState>(reader.ReadByte());
        }

        void Write(Writer& writer) const
        {
            writer.WriteByte(num(this->_currentState));
        }

      private:
        SailOpeningState _currentState;
    };
}

#endif /* LIBS_STATE_INCLUDE_STATE_SAIL_SAILSTATE_HPP_ */
