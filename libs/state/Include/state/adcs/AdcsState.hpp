#ifndef LIBS_STATE_INCLUDE_STATE_ADCS_ADCSSTATE_HPP_
#define LIBS_STATE_INCLUDE_STATE_ADCS_ADCSSTATE_HPP_

#pragma once

#include <cstdint>
#include "base/fwd.hpp"

namespace state
{
    /**
     * @brief This type represents persistent state of adcs subsystem.
     * @persistent_state
     */
    class AdcsState final
    {
      public:
        /**
         * @brief ctor.
         */
        AdcsState();

        /**
         * @brief ctor.
         * @param[in] value New value of the internal detumbling state.
         */
        AdcsState(bool isInternalDetumblingDisabled);

        /**
         * @brief Returns flag indicating whether the internal detumbling is disabled.
         * @return True if the internal detumbling is disabled, false otherwise.
         */
        bool IsInternalDetumblingDisabled() const noexcept;

        /**
         * @brief Read the adcs state from passed reader.
         * @param[in] reader Buffer reader that should be used to read the serialized state.
         */
        void Read(Reader& reader);

        /**
         * @brief Write the adcs state to passed buffer writer object.
         * @param[in] writer Buffer writer object that should be used to write the serialized state.
         */
        void Write(Writer& writer) const;

        /**
         * @brief Returns size of the serialized state in bytes.
         * @return Size of the serialized state in bytes.
         */
        static constexpr std::uint32_t Size();

      private:
        bool internalDetumblingDisabled;
    };

    inline bool AdcsState::IsInternalDetumblingDisabled() const noexcept
    {
        return this->internalDetumblingDisabled;
    }

    constexpr std::uint32_t AdcsState::Size()
    {
        return 1;
    }
}

#endif /* LIBS_STATE_INCLUDE_STATE_ADCS_ADCSSTATE_HPP_ */
