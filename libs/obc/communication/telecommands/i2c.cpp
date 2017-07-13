#include "i2c.hpp"
#include <algorithm>
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "logger/logger.h"
#include "system.h"
#include "telecommunication/downlink.h"

using telecommunication::downlink::CorrelatedDownlinkFrame;
using telecommunication::downlink::DownlinkAPID;
using drivers::i2c::I2CResult;

namespace obc
{
    namespace telecommands
    {
        RawI2CTelecommand::RawI2CTelecommand(drivers::i2c::II2CBus& systemBus, drivers::i2c::II2CBus& payload)
            : systemBus(systemBus), //
              payload(payload)      //
        {
        }

        void RawI2CTelecommand::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto correlationId = r.ReadByte();
            auto busSelect = r.ReadByte();
            auto address = r.ReadByte();
            auto delay = r.ReadWordLE();
            auto data = r.ReadToEnd();

            CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);

            if (!r.Status())
            {
                LOG(LOG_LEVEL_ERROR, "Malformed request");
                response.PayloadWriter().WriteByte(1);
                transmitter.SendFrame(response.Frame());
                return;
            }

            drivers::i2c::II2CBus* bus;
            switch (busSelect)
            {
                case 0:
                    bus = &this->systemBus;
                    break;
                case 1:
                    bus = &this->payload;
                    break;
                default:
                    LOGF(LOG_LEVEL_ERROR, "Unknown bus %d", busSelect);
                    response.PayloadWriter().WriteByte(2);
                    transmitter.SendFrame(response.Frame());
                    return;
            }

            LOGF(LOG_LEVEL_INFO, "Sending raw data to I2C bus %d", busSelect);

            auto responseBuf = response.PayloadWriter().Reserve(CorrelatedDownlinkFrame::MaxPayloadSize);

            std::fill(responseBuf.begin(), responseBuf.end(), 0);

            auto writeResult = bus->Write(address, data);
            if (writeResult != I2CResult::OK)
            {
                LOGF(LOG_LEVEL_ERROR, "Write to bus %d failed", busSelect);
                responseBuf[0] = static_cast<uint8_t>(writeResult);
                transmitter.SendFrame(response.Frame());
                return;
            }

            System::SleepTask(std::chrono::milliseconds{delay});

            auto readResult = bus->Read(address, responseBuf.subspan(1, -1));

            responseBuf[0] = static_cast<uint8_t>(readResult);

            transmitter.SendFrame(response.Frame());
        }
    }
}
