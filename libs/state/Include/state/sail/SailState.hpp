#ifndef LIBS_STATE_INCLUDE_STATE_SAIL_SAILSTATE_HPP_
#define LIBS_STATE_INCLUDE_STATE_SAIL_SAILSTATE_HPP_

#include "base/reader.h"
#include "base/writer.h"

namespace state
{
    /**
     * @brief Sail opening state
     * @ingroup persistent_state
     */
    enum class SailOpeningState : std::uint8_t
    {
        Waiting = 0,       //!< Waiting
        Opening = 1,       //!< Opening
        OpeningStopped = 2 //!< OpeningStopped
    };

    /**
     * @brief Sail state
     * @ingroup persistent_state
     * @persistent_state
     */
    class SailState
    {
      public:
        /**
         * @brief Ctor
         */
        SailState() : _currentState(SailOpeningState::Waiting)
        {
        }

        /**
         * @brief Ctor
         * @param currentState Current sail state
         */
        SailState(SailOpeningState currentState) : _currentState(currentState)
        {
        }

        /**
         * @brief Returns current sail state
         * @return Current sail state
         */
        SailOpeningState CurrentState() const
        {
            return this->_currentState;
        }

        /**
         * @brief Returns size of this object in persistent state
         * @return Size in bytes
         */
        static constexpr std::size_t Size()
        {
            return sizeof(_currentState);
        }

        /**
         * @brief Reads object from external buffer
         * @param reader Reader to use with buffer
         */
        void Read(Reader& reader)
        {
            this->_currentState = static_cast<SailOpeningState>(reader.ReadByte());
        }

        /**
         * @brief Writes object to external buffer
         * @param writer Writer to use with buffer
         */
        void Write(Writer& writer) const
        {
            writer.WriteByte(num(this->_currentState));
        }

      private:
        /** @brief Current sail state */
        SailOpeningState _currentState;
    };
}

#endif /* LIBS_STATE_INCLUDE_STATE_SAIL_SAILSTATE_HPP_ */
