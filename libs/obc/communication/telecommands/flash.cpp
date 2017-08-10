#include "flash.hpp"

using EraseStatus = experiment::erase_flash::Status;
using telecommunication::downlink::CorrelatedDownlinkFrame;
using telecommunication::downlink::DownlinkAPID;

namespace obc
{
    namespace telecommands
    {
        EraseFlashTelecommand::EraseFlashTelecommand(experiments::IExperimentController& experiments) : _experiments(experiments)
        {
        }

        void EraseFlashTelecommand::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            auto correlationId = 0;

            if (parameters.size() == 0)
            {
                CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);
                response.PayloadWriter().WriteByte(1);
                transmitter.SendFrame(response.Frame());

                return;
            }

            correlationId = parameters[0];
            CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);

            auto requested = this->_experiments.RequestExperiment(7);

            if (requested)
            {
                response.PayloadWriter().WriteByte(0);
                response.PayloadWriter().WriteByte(num(EraseStatus::Requested));
            }
            else
            {
                response.PayloadWriter().WriteByte(2);
            }

            transmitter.SendFrame(response.Frame());
        }
    }
}
