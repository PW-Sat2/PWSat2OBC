#include "fdir.hpp"
#include "state/struct.h"

namespace mission
{
    SaveErrorCountersConfig::SaveErrorCountersConfig(IGetErrorCounterConfig& getConfig) : _getConfig(getConfig)
    {
    }

    UpdateDescriptor<SystemState> SaveErrorCountersConfig::BuildUpdate()
    {
        UpdateDescriptor<SystemState> update;

        update.name = "FDIR: save error counters config";
        update.param = this;
        update.updateProc = Update;

        return update;
    }

    UpdateResult SaveErrorCountersConfig::Update(SystemState& state, void* param)
    {
        auto This = static_cast<SaveErrorCountersConfig*>(param);

        state::ErrorCountersConfigState config(This->_getConfig.GetConfig());

        auto r = state.PersistentState.Set(config);

        return r ? UpdateResult::Ok : UpdateResult::Warning;
    }
}
