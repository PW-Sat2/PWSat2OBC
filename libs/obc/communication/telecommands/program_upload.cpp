#include "program_upload.hpp"
#include <array>
#include <bitset>
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "logger/logger.h"
#include "program_flash/boot_table.hpp"
#include "telecommunication/downlink.h"

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
                    this->_bootTable.Entry(i + 1).Erase();
                }
            }

            telecommunication::downlink::DownlinkFrame response((telecommunication::downlink::DownlinkAPID)0x2, 1);
            response.PayloadWriter().WriteByte(parameters[0]);

            transmitter.SendFrame(response.Frame());
        }

        WriteProgramPart::WriteProgramPart(program_flash::BootTable& bootTable) : _bootTable(bootTable)
        {
        }

        void WriteProgramPart::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            std::bitset<8> selectedEntries(r.ReadByte());
            auto offset = r.ReadWordLE();
            auto content = r.ReadToEnd();

            if (!r.Status())
            {
                return;
            }

            LOGF(LOG_LEVEL_INFO, "Uploading program part %d to 0x%X", content.size(), offset);

            for (auto i = 0; i < 7; i++)
            {
                if (selectedEntries[i])
                {
                    this->_bootTable.Entry(i + 1).WriteContent(offset, content);
                }
            }

            telecommunication::downlink::DownlinkFrame response((telecommunication::downlink::DownlinkAPID)0x2, 1);
            response.PayloadWriter().WriteByte(parameters[0]);
            response.PayloadWriter().WriteWordLE(offset);

            transmitter.SendFrame(response.Frame());
        }

        FinalizeProgramEntry::FinalizeProgramEntry(program_flash::BootTable& bootTable) : _bootTable(bootTable)
        {
        }

        void FinalizeProgramEntry::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            std::bitset<8> selectedEntries(r.ReadByte());
            auto length = r.ReadWordLE();
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
                    e.Crc(expectedCrc);
                    e.Length(length);
                    e.Description(description.data());
                    e.MarkAsValid();
                }
            }

            telecommunication::downlink::DownlinkFrame response((telecommunication::downlink::DownlinkAPID)0x2, 1);
            response.PayloadWriter().WriteByte(parameters[0]);

            transmitter.SendFrame(response.Frame());
        }
    }
}
