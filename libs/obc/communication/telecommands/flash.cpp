#include "flash.hpp"
#include "experiment/flash/flash.hpp"

using telecommunication::downlink::CorrelatedDownlinkFrame;
using telecommunication::downlink::DownlinkAPID;

namespace obc
{
    namespace telecommands
    {
        EraseFlashTelecommand::EraseFlashTelecommand(
            experiments::IExperimentController& experiments, experiment::erase_flash::ISetCorrelationId& setId)
            : _experiments(experiments), _setId(setId)
        {
        }

        void EraseFlashTelecommand::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            auto correlationId = 0;

            if (parameters.size() == 0)
            {
                CorrelatedDownlinkFrame response(DownlinkAPID::EraseFlash, 0, correlationId);
                response.PayloadWriter().WriteByte(1);
                transmitter.SendFrame(response.Frame());

                return;
            }

            correlationId = parameters[0];
            CorrelatedDownlinkFrame response(DownlinkAPID::EraseFlash, 0, correlationId);

            this->_setId.SetCorrelationId(correlationId);

            auto requested = this->_experiments.RequestExperiment(experiment::erase_flash::EraseFlashExperiment::Code);

            if (requested)
            {
                response.PayloadWriter().WriteByte(0);
                response.PayloadWriter().WriteByte(0);
            }
            else
            {
                response.PayloadWriter().WriteByte(2);
            }

            transmitter.SendFrame(response.Frame());
        }
    }
}
