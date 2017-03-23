#include "BeaconUpdate.hpp"
#include <chrono>
#include "comm/IBeaconController.hpp"
#include "logger/logger.h"
#include "telecommunication/FrameContentWriter.hpp"

namespace mission
{
    using namespace std::chrono_literals;
    using std::chrono::duration_cast;
    using std::chrono::seconds;

    /**
     * @brief Interval between two subsequent beacon updates
     */
    static constexpr std::chrono::milliseconds BeaconUpdateInterval = 5min;

    /**
     * @brief Default beacon send interval.
     */
    static constexpr seconds BeaconInterval = 30s;

    using telecommunication::downlink::FieldId;

    BeaconUpdate::BeaconUpdate(devices::comm::IBeaconController& beaconController) //
        : controller(&beaconController),                                           //
          lastBeaconUpdate(0s),
          frame(telecommunication::downlink::DownlinkAPID::Beacon, 0)
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

    bool BeaconUpdate::ShouldUpdateBeacon(const SystemState& state, void* param)
    {
        auto This = static_cast<BeaconUpdate*>(param);
        return state.Antenna.Deployed && //
            ((state.Time - This->lastBeaconUpdate) >= BeaconUpdateInterval);
    }

    void BeaconUpdate::Run(const SystemState& state, void* param)
    {
        auto This = static_cast<BeaconUpdate*>(param);
        This->UpdateBeacon(state);
    }

    void BeaconUpdate::UpdateBeacon(const SystemState& state)
    {
        const auto beacon = GenerateBeacon(state);
        if (this->controller->SetBeacon(beacon))
        {
            this->lastBeaconUpdate = state.Time;
            LOGF(LOG_LEVEL_INFO, "Beacon set at %lu", static_cast<std::uint32_t>(duration_cast<seconds>(state.Time).count()));
        }
        else
        {
            LOGF(LOG_LEVEL_ERROR, "Unable to set beacon at %lu", static_cast<std::uint32_t>(duration_cast<seconds>(state.Time).count()));
        }
    }

    devices::comm::Beacon BeaconUpdate::GenerateBeacon(const SystemState& state)
    {
        // TODO update & move it to separate controller
        telecommunication::downlink::FrameContentWriter writer(frame.PayloadWriter());
        writer.Reset();
        writer.WriteQuadWordLE(FieldId::TimeStamp, state.Time.count());

        // TODO beacon interval will probably be adjusted based on current satelite state.
        return devices::comm::Beacon(BeaconInterval, frame.Frame());
    }
}
