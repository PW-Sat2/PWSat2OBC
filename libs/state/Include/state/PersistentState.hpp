#ifndef LIBS_STATE_PERSISTENT_STATE_HPP
#define LIBS_STATE_PERSISTENT_STATE_HPP

#pragma once

#include <cstdint>
#include "antenna/AntennaConfiguration.hpp"

namespace state
{
    class PersistentState
    {
      public:
        void Read(Reader& reader);

        void Write(Writer& writer) const;

        static constexpr std::uint32_t Size();

      private:
        AntennaConfiguration antennaConfiguration;
    };

    inline constexpr std::uint32_t PersistentState::Size()
    {
        return decltype(antennaConfiguration)::Size();
    }
}

#endif
