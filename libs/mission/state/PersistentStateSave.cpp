#include "PersistentStateSave.hpp"

namespace mission
{
    /**
     * @brief ctor.
     * @param[in] arguments Reference to time providier argument list.
     */
    PeristentStateSave::PeristentStateSave(std::tuple<obc::IStorageAccess&, std::uint32_t> arguments)
        : storageAccess(std::get<0>(arguments)), //
          baseAddress(std::get<1>(arguments))
    {
    }

    /**
     * @brief Builds action descriptor for this task.
     * @return Action descriptor - the time correction task.
     */
    ActionDescriptor<SystemState> PeristentStateSave::BuildAction()
    {
        ActionDescriptor<SystemState> result;
        result.actionProc = SaveState;
        result.condition = SaveStateCondition;
        result.name = "PersistentStateSave";
        result.param = this;
        return result;
    }

    bool PeristentStateSave::SaveStateCondition(const SystemState& state, void* param)
    {
        UNREFERENCED_PARAMETER(param);
        return state.PersistentState.IsModified();
    }

    void PeristentStateSave::SaveState(const SystemState& state, void* param)
    {
        auto This = static_cast<PeristentStateSave*>(param);
        This->SaveState(state);
    }

    void PeristentStateSave::SaveState(const SystemState& state)
    {
        obc::WritePersistentState(state.PersistentState, this->baseAddress, this->storageAccess);
    }
}
