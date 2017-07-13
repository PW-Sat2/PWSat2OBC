#include "i2c.hpp"
#include <algorithm>
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "logger/logger.h"
#include "system.h"
#include "telecommunication/downlink.h"

using telecommunication::downlink::CorrelatedDownlinkFrame;
using telecommunication::downlink::DownlinkAPID;

namespace obc
{
    namespace telecommands
    {
        RawI2CTelecommand::RawI2CTelecommand(drivers::i2c::II2CBus& i2cBus) : i2cBus(i2cBus)
        {
        }

        void RawI2CTelecommand::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto correlationId = r.ReadByte();
            auto address = r.ReadByte();
            auto data = r.ReadToEnd();

            CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);

            LOG(LOG_LEVEL_INFO, "Sending raw data to I2C bus");

            auto resultBuf = response.PayloadWriter().Reserve(1);
            auto responseBuf = response.PayloadWriter().Reserve(RawI2CTelecommand::MaxDataSize);

            std::fill(responseBuf.begin(), responseBuf.end(), 0);

            auto result = this->i2cBus.WriteRead(address, data, responseBuf);
            resultBuf[0] = static_cast<uint8_t>(result);

            transmitter.SendFrame(response.Frame());
        }
    }
}
