#include "sender.hpp"
#include "base/IHasState.hpp"
#include "base/os.h"
#include "comm/ITransmitter.hpp"
#include "logger/logger.h"
#include "telecommunication/beacon.hpp"
#include "telemetry/state.hpp"

using namespace std::chrono_literals;

namespace beacon
{
    BeaconSender::BeaconSender(devices::comm::ITransmitter& transmitter, IHasState<telemetry::TelemetryState>& telemetry)
        : _transmitter(transmitter), _telemetry(telemetry)
    {
    }

    void BeaconSender::RunOnce()
    {
        LOG(LOG_LEVEL_INFO, "Send beacon!");

        auto telemetry = this->_telemetry.GetState();

        std::chrono::seconds beaconDelay;

        if (!WriteBeaconPayload(telemetry, this->_frame.PayloadWriter()))
        {
            beaconDelay = 5s;
        }
        else
        {
            beaconDelay = 60s;
        }

        auto frame = this->_frame.Frame();

        if (frame.size() > 0)
        {
            if (!this->_transmitter.SendFrame(this->_frame.Frame()))
            {
                LOG(LOG_LEVEL_ERROR, "Beacon send failure");
                beaconDelay = 5s;
            }
        }

        System::SleepTask(beaconDelay);
    }
}
