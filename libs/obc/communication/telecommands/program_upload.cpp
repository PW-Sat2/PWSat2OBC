#include "program_upload.hpp"
#include <array>
#include <bitset>
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "logger/logger.h"
#include "program_flash/boot_table.hpp"
#include "telecommunication/downlink.h"

using telecommunication::downlink::DownlinkAPID;
using telecommunication::downlink::DownlinkFrame;
using program_flash::FlashStatus;

namespace obc
{
    namespace telecommands
    {
        EraseBootTableEntry::EraseBootTableEntry(program_flash::BootTable& bootTable) : _bootTable(bootTable)
        {
        }

        void EraseBootTableEntry::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            if (parameters.size() != 1)
            {
                return;
            }

            std::bitset<8> selectedEntries(parameters[0]);

            for (auto i = 0; i < 7; i++)
            {
                if (selectedEntries[i])
                {
                    auto result = this->_bootTable.Entry(i + 1).Erase();

                    if (!result)
                    {
                        DownlinkFrame response(DownlinkAPID::ProgramUpload, 0);
                        auto& writer = response.PayloadWriter();
                        writer.WriteByte(0);
                        writer.WriteByte(0x00);
                        writer.WriteByte(static_cast<std::uint8_t>(std::get<0>(result.Error())));
                        writer.WriteByte(1 << i);
                        writer.WriteDoubleWordLE(std::get<1>(result.Error()));

                        transmitter.SendFrame(response.Frame());
                        return;
                    }
                }
            }

            DownlinkFrame response(DownlinkAPID::ProgramUpload, 0);
            auto& writer = response.PayloadWriter();
            writer.WriteByte(0);
            writer.WriteByte(0xFF);
            writer.WriteByte(parameters[0]);

            transmitter.SendFrame(response.Frame());
        }

        WriteProgramPart::WriteProgramPart(program_flash::BootTable& bootTable) : _bootTable(bootTable)
        {
        }

        void WriteProgramPart::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            std::bitset<8> selectedEntries(r.ReadByte());
            auto offset = r.ReadDoubleWordLE();
            auto content = r.ReadToEnd();

            if (!r.Status())
            {
                return;
            }

            LOGF(LOG_LEVEL_INFO, "Uploading program part %d to 0x%lX", content.size(), offset);

            for (auto i = 0; i < 7; i++)
            {
                if (selectedEntries[i])
                {
                    auto r = this->_bootTable.Entry(i + 1).WriteContent(offset, content);

                    if (r != FlashStatus::NotBusy)
                    {
                        DownlinkFrame response(DownlinkAPID::ProgramUpload, 0);
                        auto& writer = response.PayloadWriter();
                        writer.WriteByte(1);
                        writer.WriteByte(0x0);
                        writer.WriteByte(static_cast<std::uint8_t>(r));
                        writer.WriteByte(1 << i);
                        writer.WriteDoubleWordLE(offset);

                        transmitter.SendFrame(response.Frame());
                        return;
                    }
                }
            }

            DownlinkFrame response(DownlinkAPID::ProgramUpload, 0);
            auto& writer = response.PayloadWriter();
            writer.WriteByte(1);
            writer.WriteByte(0xFF);
            writer.WriteByte(parameters[0]);
            writer.WriteDoubleWordLE(offset);
            writer.WriteByte(content.size());

            transmitter.SendFrame(response.Frame());
        }

        FinalizeProgramEntry::FinalizeProgramEntry(program_flash::BootTable& bootTable) : _bootTable(bootTable)
        {
        }

        constexpr FlashStatus Worst(FlashStatus a, FlashStatus b)
        {
            if (a == FlashStatus::NotBusy)
                return b;

            return a;
        }

        void FinalizeProgramEntry::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            std::bitset<8> selectedEntries(r.ReadByte());
            auto length = r.ReadDoubleWordLE();
            auto expectedCrc = r.ReadWordLE();

            std::array<char, 30> description{{0}};
            auto tmp = r.ReadToEnd();
            std::copy(tmp.begin(), tmp.end(), description.begin());

            if (!r.Status())
            {
                return;
            }

            LOG(LOG_LEVEL_INFO, "Finalizing entries");

            for (auto i = 0; i < 7; i++)
            {
                if (selectedEntries[i])
                {
                    auto e = this->_bootTable.Entry(i + 1);

                    FlashStatus r = FlashStatus::NotBusy;

                    r = Worst(r, e.Crc(expectedCrc));
                    r = Worst(r, e.Length(length));
                    r = Worst(r, e.Description(description.data()));
                    r = Worst(r, e.MarkAsValid());

                    if (r != FlashStatus::NotBusy)
                    {
                        DownlinkFrame response(DownlinkAPID::ProgramUpload, 0);
                        auto& writer = response.PayloadWriter();
                        writer.WriteByte(2);
                        writer.WriteByte(0x0);
                        writer.WriteByte(static_cast<std::uint8_t>(r));
                        writer.WriteByte(1 << i);

                        transmitter.SendFrame(response.Frame());
                        return;
                    }

                    auto actualCrc = e.CalculateCrc();

                    if (actualCrc != expectedCrc)
                    {
                        DownlinkFrame response(DownlinkAPID::ProgramUpload, 0);
                        auto& writer = response.PayloadWriter();
                        writer.WriteByte(2);
                        writer.WriteByte(0x01);
                        writer.WriteByte(1 << i);
                        writer.WriteWordLE(actualCrc);

                        transmitter.SendFrame(response.Frame());
                        return;
                    }
                }
            }

            DownlinkFrame response(DownlinkAPID::ProgramUpload, 0);
            auto& writer = response.PayloadWriter();
            writer.WriteByte(2);
            writer.WriteByte(0xFF);
            writer.WriteByte(parameters[0]);
            writer.WriteWordLE(expectedCrc);

            transmitter.SendFrame(response.Frame());
        }
    }
}
