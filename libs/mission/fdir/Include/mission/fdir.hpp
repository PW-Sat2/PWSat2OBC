#ifndef LIBS_MISSION_FDIR_INCLUDE_MISSION_FDIR_HPP_
#define LIBS_MISSION_FDIR_INCLUDE_MISSION_FDIR_HPP_

#include <array>
#include "error_counter/error_counter.hpp"
#include "mission/base.hpp"
#include "state/fwd.hpp"

namespace mission
{
    /**
     * @brief Interface that can be used to retrieve current error counters config
     * @ingroup mission
     */
    struct IGetErrorCounterConfig
    {
        /**
         * @brief Returns current error counter config
         * @return Opaque buffer with current error counter config
         */
        virtual std::array<std::uint32_t, error_counter::ErrorCounting::MaxDevices> GetConfig() = 0;
    };

    /**
     * @brief Mission component responsible for periodic dump of error counters config
     * @mission_task
     * @ingroup mission
     */
    class SaveErrorCountersConfig : public mission::Update
    {
      public:
        /**
         * @brief Ctor
         * @param getConfig Reference to object that can be used to get current error counter config
         */
        SaveErrorCountersConfig(IGetErrorCounterConfig& getConfig);

        /**
         * @brief Builds mission update description
         * @return Update description
         */
        mission::UpdateDescriptor<SystemState> BuildUpdate();

      private:
        /**
         * @brief Updates persistent state with current error counter config
         * @param state System state
         * @param param Pointer to object of this type
         * @return Update result
         */
        static UpdateResult Update(SystemState& state, void* param);

        /** @brief Reference to object that can be used to get current error counter config */
        IGetErrorCounterConfig& _getConfig;
    };
}

#endif /* LIBS_MISSION_FDIR_INCLUDE_MISSION_FDIR_HPP_ */
