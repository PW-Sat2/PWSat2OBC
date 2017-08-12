#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_SUNS_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_SUNS_HPP_

#include "base/IHasState.hpp"
#include "experiment/suns/suns.hpp"
#include "gyro/gyro.h"
#include "gyro/telemetry.hpp"
#include "payload/interfaces.h"
#include "power/fwd.hpp"
#include "state/struct.h"
#include "suns/suns.hpp"
#include "telecommunication/downlink.h"
#include "telecommunication/telecommand_handling.h"
#include "time/fwd.hpp"

namespace obc
{
    namespace telecommands
    {
        /**
         * @brief Retrieve SunS data sets
         * @ingroup telecommands
         * @telecommand
         *
         * Command code: 0xD0
         *
         * Parameters:
         *  - 8-bit - Correlation id
         *  - 8-bit - Gain
         *  - 8-bit - Itime
         *
         * Returned data:
         *  - Timestamp,
         *  - Experimental suns primary data
         *  - Reference suns data
         *  - Gyro data
         *  - Experimental suns secondary data
         */
        class GetSunSDataSetsTelecommand final : public telecommunication::uplink::Telecommand<0xD0>
        {
          public:
            /**
             * @brief Ctor
             * @param[in] powerControl Power control interface
             * @param[in] currentTime Current time
             * @param[in] experimentalSunS Experimental Sun Sensor interface
             * @param[in] payload Payload interface
             * @param[in] gyro Gyroscope interface
             */
            GetSunSDataSetsTelecommand(                          //
                services::power::IPowerControl& powerControl,    //
                services::time::ICurrentTime& currentTime,       //
                devices::suns::ISunSDriver& experimentalSunS,    //
                devices::payload::IPayloadDeviceDriver& payload, //
                devices::gyro::IGyroscopeDriver& gyro);          //

            /**
             * @brief Method called when telecommand is received.
             * @param[in] transmitter Reference to object that can be used to send response back
             * @param[in] parameters Parameters contained in telecommand frame
             */
            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            services::power::IPowerControl& _powerControl;
            services::time::ICurrentTime& _currentTime;
            devices::suns::ISunSDriver& _experimentalSunS;
            devices::payload::IPayloadDeviceDriver& _payload;
            devices::gyro::IGyroscopeDriver& _gyro;

            void GatherSingleMeasurement(experiment::suns::DataPoint& point, uint8_t gain, uint8_t itime);
            void WriteMeasurements(experiment::suns::DataPoint& point, Writer& writer);
            void SetPowerState(bool state);
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_SUNS_HPP_ */
