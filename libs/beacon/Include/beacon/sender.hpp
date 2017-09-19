#ifndef LIBS_BEACON_INCLUDE_BEACON_SENDER_HPP_
#define LIBS_BEACON_INCLUDE_BEACON_SENDER_HPP_

#include "base/fwd.hpp"
#include "comm/comm.hpp"
#include "telecommunication/downlink.h"
#include "telemetry/fwd.hpp"

namespace beacon
{
    /**
     * @brief Beacon sender
     *
     * This class is responsible for sending beacon
     */
    class BeaconSender
    {
      public:
        /**
         * @brief Ctor
         * @param transmitter Frame transmitter
         * @param telemetry Telemetry state accessor
         */
        BeaconSender(devices::comm::ITransmitter& transmitter, IHasState<telemetry::TelemetryState>& telemetry);

        /**
         * @brief Run single iteration
         */
        void RunOnce();

      private:
        /** @brief Transmitter */
        devices::comm::ITransmitter& _transmitter;
        /** @brief Telemetry state accessor */
        IHasState<telemetry::TelemetryState>& _telemetry;
        /** @brief Beacon frame */
        telecommunication::downlink::RawFrame _frame;
    };
}

#endif /* LIBS_BEACON_INCLUDE_BEACON_SENDER_HPP_ */
