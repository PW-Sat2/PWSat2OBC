#include "fdir.hpp"
#include <type_traits>
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "telecommunication/downlink.h"

using telecommunication::downlink::DownlinkAPID;

using telecommunication::downlink::CorrelatedDownlinkFrame;

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

            CorrelatedDownlinkFrame responseFrame(DownlinkAPID::Operation, 0, correlationId);
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
    }
}