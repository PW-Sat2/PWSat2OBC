#include "fdir.hpp"
#include <type_traits>
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "telecommunication/downlink.h"

using telecommunication::downlink::DownlinkAPID;
using telecommunication::downlink::CorrelatedDownlinkFrame;
using telecommunication::downlink::DownlinkFrame;

namespace obc
{
    namespace telecommands
    {
        static_assert(std::is_same<error_counter::CounterValue, std::uint8_t>::value,
            "Mismatch between error counter value type and telecommand format");

        SetErrorCounterConfig::SetErrorCounterConfig(ISetErrorCounterConfig& config) : _config(config)
        {
        }

        void SetErrorCounterConfig::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto correlationId = r.ReadByte();

            CorrelatedDownlinkFrame responseFrame(DownlinkAPID::ErrorCounterConfiguration, 0, correlationId);
            auto& response = responseFrame.PayloadWriter();

            if (!r.Status())
            {
                response.WriteByte(1);
                transmitter.SendFrame(responseFrame.Frame());
                return;
            }

            response.WriteByte(0);

            while (true)
            {
                auto device = r.ReadByte();
                auto limit = r.ReadByte();
                auto increment = r.ReadByte();
                auto decrement = r.ReadByte();

                if (!r.Status())
                {
                    break;
                }

                if (device > error_counter::ErrorCounting::MaxDevices)
                {
                    break;
                }

                this->_config.Set(device, limit, increment, decrement);

                response.WriteByte(device);
            }

            transmitter.SendFrame(responseFrame.Frame());
        }

        static_assert(4 * sizeof(error_counter::CounterValue) * error_counter::ErrorCounting::MaxDevices < DownlinkFrame::MaxPayloadSize,
            "Config to large to fit into single frame");

        GetErrorCountersConfigTelecommand::GetErrorCountersConfigTelecommand(
            error_counter::IErrorCounting& errorCounting, error_counter::IErrorCountingConfigration& config)
            : _errorCounting(errorCounting), _config(config)
        {
        }

        void GetErrorCountersConfigTelecommand::Handle(
            devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> /*parameters*/)
        {
            DownlinkFrame response(DownlinkAPID::ErrorCounters, 0);

            auto& writer = response.PayloadWriter();

            for (auto i = 0; i < error_counter::ErrorCounting::MaxDevices; i++)
            {
                writer.WriteByte(_errorCounting.Current(i));
                writer.WriteByte(_config.Limit(i));
                writer.WriteByte(_config.Increment(i));
                writer.WriteByte(_config.Decrement(i));
            }

            transmitter.SendFrame(response.Frame());
        }
    }
}
