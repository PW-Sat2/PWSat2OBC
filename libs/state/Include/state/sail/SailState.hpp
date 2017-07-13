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
        SailState();

        /**
         * @brief Ctor
         * @param currentState Current sail state
         */
        SailState(SailOpeningState currentState);

        /**
         * @brief Returns current sail state
         * @return Current sail state
         */
        SailOpeningState CurrentState() const;

        /**
         * @brief Returns size of this object in persistent state
         * @return Size in bytes
         */
        static constexpr std::size_t Size();

        /**
         * @brief Reads object from external buffer
         * @param reader Reader to use with buffer
         */
        void Read(Reader& reader);

        /**
         * @brief Writes object to external buffer
         * @param writer Writer to use with buffer
         */
        void Write(Writer& writer) const;

      private:
        /** @brief Current sail state */
        SailOpeningState _currentState;
    };

    inline state::SailOpeningState SailState::CurrentState() const
    {
        return this->_currentState;
    }

    inline constexpr std::size_t SailState::Size()
    {
        return sizeof(_currentState);
    }
}

#endif /* LIBS_STATE_INCLUDE_STATE_SAIL_SAILSTATE_HPP_ */
