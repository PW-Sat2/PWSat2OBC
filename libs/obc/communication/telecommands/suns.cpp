#include "suns.hpp"
#include <chrono>
#include "base/os.h"
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "experiment/suns/suns.hpp"
#include "logger/logger.h"
#include "power/power.h"
#include "telecommunication/downlink.h"
#include "telecommunication/telecommand_handling.h"
#include "time/ICurrentTime.hpp"

using namespace std::literals;

using telecommunication::downlink::CorrelatedDownlinkFrame;
using telecommunication::downlink::DownlinkAPID;
using experiment::suns::DataPoint;

namespace obc
{
    namespace telecommands
    {
        GetSunSDataSetsTelecommand::GetSunSDataSetsTelecommand( //
            services::power::IPowerControl& powerControl,       //
            services::time::ICurrentTime& currentTime,          //
            devices::suns::ISunSDriver& experimentalSunS,       //
            devices::payload::IPayloadDeviceDriver& payload,    //
            devices::gyro::IGyroscopeDriver& gyro)              //
            : _powerControl(powerControl),
              _currentTime(currentTime),
              _experimentalSunS(experimentalSunS),
              _payload(payload),
              _gyro(gyro)
        {
        }

        void GetSunSDataSetsTelecommand::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader reader(parameters);
            uint8_t correlationId = reader.ReadByte();
            uint8_t gain = reader.ReadByte();
            uint8_t itime = reader.ReadByte();

            if (!reader.Status())
            {
                LOG(LOG_LEVEL_INFO, "[tc] Failed to command read parameters");
                SendErrorFrame(transmitter, correlationId);
                return;
            }

            SetPowerState(true);
            System::SleepTask(3s);

            DataPoint point;
            this->GatherSingleMeasurement(point, gain, itime);

            SetPowerState(false);

            CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);
            WriteMeasurements(point, response.PayloadWriter());

            transmitter.SendFrame(response.Frame());
        }

        void GetSunSDataSetsTelecommand::GatherSingleMeasurement(DataPoint& point, uint8_t gain, uint8_t itime)
        {
            point.Timestamp = this->_currentTime.GetCurrentTime().Value;

            this->_experimentalSunS.StartMeasurement(gain, itime);

            this->_payload.MeasureSunSRef(point.ReferenceSunS);

            this->_experimentalSunS.WaitForData();

            this->_experimentalSunS.GetMeasuredData(point.ExperimentalSunS);

            point.Gyro = this->_gyro.read().Value;
        }

        void GetSunSDataSetsTelecommand::WriteMeasurements(DataPoint& point, Writer& writer)
        {
            point.WriteTimeStamp(writer);
            point.WritePrimaryExperimentalSunS(writer);
            point.ReferenceSunS.Write(writer);
            point.WriteGyro(writer);
            point.WriteSecondaryExperimentalSunS(writer);
        }

        void GetSunSDataSetsTelecommand::SetPowerState(bool state)
        {
            this->_powerControl.SensPower(state);
            this->_powerControl.SunSPower(state);
        }

        void GetSunSDataSetsTelecommand::SendErrorFrame(devices::comm::ITransmitter& transmitter, uint8_t correlationId)
        {
            CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);
            response.PayloadWriter().WriteByte(0x01);

            transmitter.SendFrame(response.Frame());
        }
    }
}
