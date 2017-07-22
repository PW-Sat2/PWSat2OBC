#ifndef LIBS_STATE_INCLUDE_STATE_FDIR_ERRORCOUNTERSSTATE_HPP_
#define LIBS_STATE_INCLUDE_STATE_FDIR_ERRORCOUNTERSSTATE_HPP_

#include <array>
#include "base/fwd.hpp"
#include "error_counter/error_counter.hpp"

namespace state
{
    class ErrorCountersConfigState
    {
      public:
        using ConfigBuffer = std::array<std::uint32_t, error_counter::ErrorCounting::MaxDevices>;

        ErrorCountersConfigState();

        ErrorCountersConfigState(ConfigBuffer& currentConfig);

        static constexpr std::size_t Size();

        void Read(Reader& reader);

        void Write(Writer& writer) const;

      public:
        ConfigBuffer _config;
    };

    inline constexpr std::size_t ErrorCountersConfigState::Size()
    {
        return error_counter::ErrorCounting::MaxDevices * sizeof(decltype(_config)::value_type);
    }
}

#endif /* LIBS_STATE_INCLUDE_STATE_FDIR_ERRORCOUNTERSSTATE_HPP_ */
