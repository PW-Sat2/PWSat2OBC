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
using std::get;

namespace obc
{
    namespace telecommands
    {
        static inline DownlinkFrame EraseEntryError(std::uint8_t errorCode, std::uint8_t entry, std::uint32_t offset)
        {
            DownlinkFrame frame(DownlinkAPID::ProgramUpload, 0);
            auto& writer = frame.PayloadWriter();
            writer.WriteByte(0);
            writer.WriteByte(1);
            writer.WriteByte(errorCode);
            writer.WriteByte(1 << entry);
            writer.WriteDoubleWordLE(offset);

            return frame;
        }

        static inline DownlinkFrame EraseEntryMalformedError()
        {
            DownlinkFrame frame(DownlinkAPID::ProgramUpload, 0);
            auto& writer = frame.PayloadWriter();
            writer.WriteByte(0);
            writer.WriteByte(1);
            writer.WriteByte(10);

            return frame;
        }

        static inline DownlinkFrame EraseEntrySuccess(std::uint8_t entries)
        {
            DownlinkFrame response(DownlinkAPID::ProgramUpload, 0);
            auto& writer = response.PayloadWriter();
            writer.WriteByte(0);
            writer.WriteByte(0);
            writer.WriteByte(entries);
            return response;
        }

        static inline DownlinkFrame WriteProgramError(std::uint8_t errorCode, std::uint8_t entry, std::uint32_t offset)
        {
            DownlinkFrame response(DownlinkAPID::ProgramUpload, 0);
            auto& writer = response.PayloadWriter();
            writer.WriteByte(1);
            writer.WriteByte(1);
            writer.WriteByte(errorCode);
            writer.WriteByte(1 << entry);
            writer.WriteDoubleWordLE(offset);
            return response;
        }

        static inline DownlinkFrame WriteProgramMalformedError()
        {
            DownlinkFrame frame(DownlinkAPID::ProgramUpload, 0);
            auto& writer = frame.PayloadWriter();
            writer.WriteByte(1);
            writer.WriteByte(1);
            writer.WriteByte(10);

            return frame;
        }

        static inline DownlinkFrame WriteProgramSuccess(std::uint8_t entries, std::uint32_t offset, std::uint8_t size)
        {
            DownlinkFrame response(DownlinkAPID::ProgramUpload, 0);
            auto& writer = response.PayloadWriter();
            writer.WriteByte(1);
            writer.WriteByte(0);
            writer.WriteByte(entries);
            writer.WriteDoubleWordLE(offset);
            writer.WriteByte(size);

            return response;
        }

        static inline DownlinkFrame FinalizeEntryWriteError(std::uint8_t errorCode, std::uint8_t entry)
        {
            DownlinkFrame response(DownlinkAPID::ProgramUpload, 0);
            auto& writer = response.PayloadWriter();
            writer.WriteByte(2);
            writer.WriteByte(1);
            writer.WriteByte(errorCode);
            writer.WriteByte(1 << entry);

            return response;
        }

        static inline DownlinkFrame FinalizeEntryCRCError(std::uint8_t entry, std::uint16_t actualCrc)
        {
            DownlinkFrame response(DownlinkAPID::ProgramUpload, 0);
            auto& writer = response.PayloadWriter();
            writer.WriteByte(2);
            writer.WriteByte(20);
            writer.WriteByte(1 << entry);
            writer.WriteWordLE(actualCrc);

            return response;
        }

        static inline DownlinkFrame FinalizeEntrySuccess(std::uint8_t entries, std::uint16_t crc)
        {
            DownlinkFrame response(DownlinkAPID::ProgramUpload, 0);
            auto& writer = response.PayloadWriter();
            writer.WriteByte(2);
            writer.WriteByte(0);
            writer.WriteByte(entries);
            writer.WriteWordLE(crc);

            return response;
        }
        static inline DownlinkFrame FinalizeEntryMalformedError()
        {
            DownlinkFrame frame(DownlinkAPID::ProgramUpload, 0);
            auto& writer = frame.PayloadWriter();
            writer.WriteByte(2);
            writer.WriteByte(1);
            writer.WriteByte(10);

            return frame;
        }

        EraseBootTableEntry::EraseBootTableEntry(program_flash::BootTable& bootTable) : _bootTable(bootTable)
        {
        }

        void EraseBootTableEntry::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            if (parameters.size() != 1)
            {
                transmitter.SendFrame(EraseEntryMalformedError().Frame());

                return;
            }

            std::bitset<program_flash::BootTable::EntriesCount> selectedEntries(parameters[0]);

            UniqueLock<program_flash::BootTable> lock(this->_bootTable, InfiniteTimeout);

            for (auto i = 0; i < program_flash::BootTable::EntriesCount; i++)
            {
                if (selectedEntries[i])
                {
                    auto result = this->_bootTable.Entry(i).Erase();

                    if (!result)
                    {
                        transmitter.SendFrame(EraseEntryError(num(get<0>(result.Error())), i, get<1>(result.Error())).Frame());
                        return;
                    }
                }
            }

            transmitter.SendFrame(EraseEntrySuccess(parameters[0]).Frame());
        }

        WriteProgramPart::WriteProgramPart(program_flash::BootTable& bootTable) : _bootTable(bootTable)
        {
        }

        void WriteProgramPart::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            std::bitset<program_flash::BootTable::EntriesCount> selectedEntries(r.ReadByte());
            auto offset = r.ReadDoubleWordLE();
            auto content = r.ReadToEnd();

            if (!r.Status())
            {
                transmitter.SendFrame(WriteProgramMalformedError().Frame());
                return;
            }

            LOGF(LOG_LEVEL_INFO, "Uploading program part %d to 0x%lX", content.size(), offset);

            UniqueLock<program_flash::BootTable> lock(this->_bootTable, InfiniteTimeout);

            for (auto i = 0; i < program_flash::BootTable::EntriesCount; i++)
            {
                if (selectedEntries[i])
                {
                    auto r = this->_bootTable.Entry(i).WriteContent(offset, content);

                    if (r != FlashStatus::NotBusy)
                    {
                        transmitter.SendFrame(WriteProgramError(num(r), i, offset).Frame());
                        return;
                    }
                }
            }

            transmitter.SendFrame(WriteProgramSuccess(parameters[0], offset, content.size()).Frame());
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

            std::bitset<program_flash::BootTable::EntriesCount> selectedEntries(r.ReadByte());
            auto length = r.ReadDoubleWordLE();
            auto expectedCrc = r.ReadWordLE();

            std::array<char, 30> description{{0}};
            auto tmp = r.ReadToEnd();
            std::copy(tmp.begin(), tmp.end(), description.begin());

            if (!r.Status())
            {
                transmitter.SendFrame(FinalizeEntryMalformedError().Frame());
                return;
            }

            LOG(LOG_LEVEL_INFO, "Finalizing entries");

            UniqueLock<program_flash::BootTable> lock(this->_bootTable, InfiniteTimeout);

            for (auto i = 0; i < program_flash::BootTable::EntriesCount; i++)
            {
                if (selectedEntries[i])
                {
                    auto e = this->_bootTable.Entry(i);

                    FlashStatus r = FlashStatus::NotBusy;

                    r = Worst(r, e.Crc(expectedCrc));
                    r = Worst(r, e.Length(length));
                    r = Worst(r, e.Description(description.data()));
                    r = Worst(r, e.MarkAsValid());

                    if (r != FlashStatus::NotBusy)
                    {
                        transmitter.SendFrame(FinalizeEntryWriteError(num(r), i).Frame());
                        return;
                    }

                    auto actualCrc = e.CalculateCrc();

                    if (actualCrc != expectedCrc)
                    {
                        transmitter.SendFrame(FinalizeEntryCRCError(i, actualCrc).Frame());
                        return;
                    }
                }
            }

            transmitter.SendFrame(FinalizeEntrySuccess(parameters[0], expectedCrc).Frame());
        }
    }
}
