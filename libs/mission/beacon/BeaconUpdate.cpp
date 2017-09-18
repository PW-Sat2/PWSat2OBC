#include "BeaconUpdate.hpp"
#include <chrono>

namespace mission
{
    BeaconUpdate::BeaconUpdate(std::uint8_t /*unused*/)
        : beaconTaskHandle(nullptr), //
          isBeaconEnabled(false)
    {
    }

    ActionDescriptor<SystemState> BeaconUpdate::BuildAction()
    {
        ActionDescriptor<SystemState> descriptor;
        descriptor.name = "Beacon Update";
        descriptor.param = this;
        descriptor.condition = ShouldUpdateBeacon;
        descriptor.actionProc = Run;
        return descriptor;
    }

    void BeaconUpdate::BeaconTaskHandle(OSTaskHandle handle)
    {
        this->beaconTaskHandle = handle;
    }

    bool BeaconUpdate::ShouldUpdateBeacon(const SystemState& state, void* param)
    {
        auto This = static_cast<BeaconUpdate*>(param);
        return state.AntennaState.IsDeployed() && This->beaconTaskHandle != nullptr && !This->isBeaconEnabled;
    }

    void BeaconUpdate::Run(SystemState& /*state*/, void* param)
    {
        auto This = static_cast<BeaconUpdate*>(param);
        System::ResumeTask(This->beaconTaskHandle);
        This->isBeaconEnabled = true;
    }
}
