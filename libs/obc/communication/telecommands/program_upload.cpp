#include "program_upload.hpp"
#include <array>
#include <bitset>
#include "base/reader.h"
#include "comm/ITransmitFrame.hpp"
#include "logger/logger.h"
#include "program_flash/boot_table.hpp"
#include "telecommunication/downlink.h"

constexpr std::uint8_t* FlashBase = reinterpret_cast<std::uint8_t*>(0x84000000);

namespace obc
{
    namespace telecommands
    {
        void EraseBootTableEntry::Handle(devices::comm::ITransmitFrame& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            if (parameters.size() != 1)
            {
                return;
            }

            std::bitset<8> selectedEntries(parameters[0]);

            program_flash::BootTable bootTable(FlashBase);

            for (auto i = 0; i < 7; i++)
            {
                if (selectedEntries[i])
                {
                    bootTable.Entry(i + 1).Erase();
                }
            }

            telecommunication::downlink::DownlinkFrame response((telecommunication::downlink::DownlinkAPID)0x2, 1);
            response.PayloadWriter().WriteByte(parameters[0]);

            transmitter.SendFrame(response.Frame());
        }

        void WriteProgramPart::Handle(devices::comm::ITransmitFrame& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            std::bitset<8> selectedEntries(r.ReadByte());
            auto offset = r.ReadWordLE();
            auto content = r.ReadToEnd();

            if (!r.Status())
            {
                return;
            }

            program_flash::BootTable bootTable(FlashBase);

            LOGF(LOG_LEVEL_INFO, "Uploading program part %d to 0x%X", content.size(), offset);

            for (auto i = 0; i < 7; i++)
            {
                if (selectedEntries[i])
                {
                    bootTable.Entry(i + 1).WriteContent(offset, content);
                }
            }

            telecommunication::downlink::DownlinkFrame response((telecommunication::downlink::DownlinkAPID)0x2, 1);
            response.PayloadWriter().WriteByte(parameters[0]);
            response.PayloadWriter().WriteWordLE(offset);

            transmitter.SendFrame(response.Frame());
        }

        void FinalizeProgramEntry::Handle(devices::comm::ITransmitFrame& transmitter, gsl::span<const std::uint8_t> parameters)
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

            program_flash::BootTable bootTable(FlashBase);

            LOG(LOG_LEVEL_INFO, "Finalizing entries");

            for (auto i = 0; i < 7; i++)
            {
                if (selectedEntries[i])
                {
                    auto e = bootTable.Entry(i + 1);
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
