#ifndef LIBS_STATE_INCLUDE_STATE_FDIR_ERRORCOUNTERSSTATE_HPP_
#define LIBS_STATE_INCLUDE_STATE_FDIR_ERRORCOUNTERSSTATE_HPP_

#include <array>
#include "base/fwd.hpp"
#include "error_counter/error_counter.hpp"

namespace state
{
    /**
     * @brief This type represents error counters config stored in persistent state
     * @persistent_state
     * @remark Information stored in objects of this type are opaque and should not be used directly
     */
    class ErrorCountersConfigState
    {
      public:
        /** @brief Type of buffer used to store error counters config */
        using ConfigBuffer = std::array<std::uint32_t, error_counter::ErrorCounting::MaxDevices>;

        /**
         * @brief Default ctor
         */
        ErrorCountersConfigState();

        /**
         * @brief Ctor
         * @param currentConfig Config to store in persistent state
         */
        ErrorCountersConfigState(const ConfigBuffer& currentConfig);

        /**
         * @brief Returns size of the serialized state in bytes.
         * @return Size of the serialized state in bytes.
         */
        static constexpr std::size_t Size();

        /**
         * @brief Reads error counters config from serialized state
         * @param reader Buffer reader that should be used to read the serialized state.
         */
        void Read(Reader& reader);

        /**
         * @brief Writes error counters config into serialized state
         * @param writer Buffer writer object that should be used to write the serialized state.
         */
        void Write(Writer& writer) const;

      public:
        /** @brief Current error counters config */
        ConfigBuffer _config;
    };

    inline constexpr std::size_t ErrorCountersConfigState::Size()
    {
        return error_counter::ErrorCounting::MaxDevices * sizeof(decltype(_config)::value_type);
    }
}

#endif /* LIBS_STATE_INCLUDE_STATE_FDIR_ERRORCOUNTERSSTATE_HPP_ */
