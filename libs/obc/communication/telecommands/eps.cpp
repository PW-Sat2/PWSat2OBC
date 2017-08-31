#include "eps.hpp"
#include <cmath>
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "eps/eps.h"
#include "logger/logger.h"
#include "system.h"
#include "telecommunication/downlink.h"

using telecommunication::downlink::CorrelatedDownlinkFrame;
using telecommunication::downlink::DownlinkAPID;

namespace obc
{
    namespace telecommands
    {
        using namespace devices::eps;

        DisableOverheatSubmodeTelecommand::DisableOverheatSubmodeTelecommand(devices::eps::IEPSDriver& epsDriver) : epsDriver(epsDriver)
        {
        }

        void DisableOverheatSubmodeTelecommand::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto correlationId = r.ReadByte();
            auto controller = static_cast<IEPSDriver::Controller>(r.ReadByte());

            CorrelatedDownlinkFrame response(DownlinkAPID::DisableOverheatSubmode, 0, correlationId);

            if (!r.Status())
            {
                LOG(LOG_LEVEL_ERROR, "Malformed request");
                response.PayloadWriter().WriteByte(-1);
                transmitter.SendFrame(response.Frame());
                return;
            }

            LOGF(LOG_LEVEL_INFO, "Disabling overheat submode for controller %d", static_cast<int>(controller));

            if (!epsDriver.DisableOverheatSubmode(controller))
            {
                LOG(LOG_LEVEL_ERROR, "Unable to disable overheat submode");
                response.PayloadWriter().WriteByte(-2);
                transmitter.SendFrame(response.Frame());
                return;
            }

            response.PayloadWriter().WriteByte(0);
            transmitter.SendFrame(response.Frame());
        }
    }
}
