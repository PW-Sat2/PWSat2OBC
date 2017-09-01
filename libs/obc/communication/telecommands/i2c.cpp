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

            CorrelatedDownlinkFrame response(DownlinkAPID::I2C, 0, correlationId);

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

            I2CResult i2cResult;
            if (delay == 0)
            {
                i2cResult = HandleJoinedWriteRead(*bus, address, data, responseBuf.subspan(1, -1));
            }
            else
            {
                i2cResult = HandleSeparateWriteRead(*bus, address, std::chrono::milliseconds{delay}, data, responseBuf.subspan(1, -1));
            }

            responseBuf[0] = static_cast<uint8_t>(i2cResult);

            if (i2cResult != I2CResult::OK)
            {
                LOGF(LOG_LEVEL_ERROR, "Write to bus %d failed", busSelect);
            }

            transmitter.SendFrame(response.Frame());
        }

        I2CResult RawI2CTelecommand::HandleJoinedWriteRead(
            drivers::i2c::II2CBus& bus, uint8_t address, gsl::span<const uint8_t> data, gsl::span<uint8_t> response)
        {
            auto result = bus.WriteRead(address, data, response);
            if (result != I2CResult::OK)
            {
                LOGF(LOG_LEVEL_ERROR, "WriteRead failed (%d)", static_cast<int>(result));
            }

            return result;
        }

        I2CResult RawI2CTelecommand::HandleSeparateWriteRead(drivers::i2c::II2CBus& bus,
            uint8_t address,
            std::chrono::milliseconds delay,
            gsl::span<const uint8_t> data,
            gsl::span<uint8_t> response)
        {
            auto writeResult = bus.Write(address, data);
            if (writeResult != I2CResult::OK)
            {
                LOGF(LOG_LEVEL_ERROR, "Write failed (%d)", static_cast<int>(writeResult));
                return writeResult;
            }

            System::SleepTask(delay);

            auto readResult = bus.Read(address, response);
            if (readResult != I2CResult::OK)
            {
                LOGF(LOG_LEVEL_ERROR, "Read failed (%d)", static_cast<int>(readResult));
            }

            return readResult;
        }
    }
}
