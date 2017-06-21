#include "boot_settings.hpp"
#include "base/reader.h"
#include "boot/settings.hpp"
#include "comm/ITransmitter.hpp"
#include "telecommunication/downlink.h"

using telecommunication::downlink::DownlinkAPID;

namespace obc
{
    namespace telecommands
    {
        SetBootSlotsTelecommand::SetBootSlotsTelecommand(boot::BootSettings& settings) : _settings(settings)
        {
        }

        void SetBootSlotsTelecommand::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);
            auto correlationId = r.ReadByte();
            auto bootSlots = r.ReadByte();
            auto failsafeBootSlots = r.ReadByte();

            if (!r.Status())
            {
                telecommunication::downlink::CorrelatedDownlinkFrame frame(DownlinkAPID::Operation, 0x0, correlationId);
                frame.PayloadWriter().WriteByte(0xE0);

                transmitter.SendFrame(frame.Frame());
                return;
            }

            if (!boot::BootSettings::IsValidBootSlot(bootSlots))
            {
                telecommunication::downlink::CorrelatedDownlinkFrame frame(DownlinkAPID::Operation, 0x0, correlationId);
                frame.PayloadWriter().WriteByte(0xE1);

                transmitter.SendFrame(frame.Frame());
                return;
            }

            if (!boot::BootSettings::IsValidBootSlot(failsafeBootSlots))
            {
                telecommunication::downlink::CorrelatedDownlinkFrame frame(DownlinkAPID::Operation, 0x0, correlationId);
                frame.PayloadWriter().WriteByte(0xE2);

                transmitter.SendFrame(frame.Frame());
                return;
            }

            UniqueLock<decltype(this->_settings)> lock(this->_settings, InfiniteTimeout);

            this->_settings.BootSlots(bootSlots);
            this->_settings.FailsafeBootSlots(failsafeBootSlots);
            this->_settings.MarkAsValid();

            telecommunication::downlink::CorrelatedDownlinkFrame frame(DownlinkAPID::Operation, 0x0, correlationId);
            frame.PayloadWriter().WriteByte(0);
            frame.PayloadWriter().WriteByte(this->_settings.BootSlots());
            frame.PayloadWriter().WriteByte(this->_settings.FailsafeBootSlots());

            transmitter.SendFrame(frame.Frame());
        }
    }
}
