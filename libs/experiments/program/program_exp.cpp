#include "program_exp.hpp"
#include <cstring>
#include <gsl/span>
#include "base/os.h"
#include "base/writer.h"
#include "comm/ITransmitter.hpp"
#include "logger/logger.h"
#include "telecommunication/downlink.h"

using experiments::IterationResult;
using experiments::StartResult;
using telecommunication::downlink::DownlinkAPID;
using telecommunication::downlink::DownlinkFrame;
using namespace std::chrono_literals;
using namespace program_flash;

namespace experiment
{
    namespace program
    {
        static inline DownlinkFrame EraseEntryError(std::uint8_t errorCode, std::uint8_t entry, std::uint32_t offset)
        {
            DownlinkFrame frame(DownlinkAPID::CopyBootTable, 0);
            auto& writer = frame.PayloadWriter();
            writer.WriteByte(0);
            writer.WriteByte(1);
            writer.WriteByte(errorCode);
            writer.WriteByte(1 << entry);
            writer.WriteDoubleWordLE(offset);

            return frame;
        }

        static inline DownlinkFrame WriteProgramError(std::uint8_t errorCode, std::uint8_t entry, std::uint32_t offset)
        {
            DownlinkFrame response(DownlinkAPID::CopyBootTable, 0);
            auto& writer = response.PayloadWriter();
            writer.WriteByte(1);
            writer.WriteByte(1);
            writer.WriteByte(errorCode);
            writer.WriteByte(1 << entry);
            writer.WriteDoubleWordLE(offset);
            return response;
        }

        static inline DownlinkFrame FinalizeEntryWriteError(std::uint8_t errorCode, std::uint8_t entry)
        {
            DownlinkFrame response(DownlinkAPID::CopyBootTable, 0);
            auto& writer = response.PayloadWriter();
            writer.WriteByte(2);
            writer.WriteByte(1);
            writer.WriteByte(errorCode);
            writer.WriteByte(1 << entry);

            return response;
        }

        static inline DownlinkFrame FinalizeEntryCRCError(std::uint8_t entry, std::uint16_t actualCrc)
        {
            DownlinkFrame response(DownlinkAPID::CopyBootTable, 0);
            auto& writer = response.PayloadWriter();
            writer.WriteByte(2);
            writer.WriteByte(20);
            writer.WriteByte(1 << entry);
            writer.WriteWordLE(actualCrc);

            return response;
        }

        CopyBootSlotsExperiment::CopyBootSlotsExperiment(program_flash::BootTable& bootTable, devices::comm::ITransmitter& transmitter)
            : _bootTable(bootTable), _transmitter(transmitter)
        {
        }

        void CopyBootSlotsExperiment::SetupEntries(BootEntriesSelector& source, BootEntriesSelector& target)
        {
            _sourceEntries = source;
            _targetEntries = target;
        }

        experiments::ExperimentCode CopyBootSlotsExperiment::Type()
        {
            return Code;
        }

        StartResult CopyBootSlotsExperiment::Start()
        {
            return StartResult::Success;
        }

        constexpr FlashStatus Worst(FlashStatus a, FlashStatus b)
        {
            if (a == program_flash::FlashStatus::NotBusy)
                return b;

            return a;
        }

        IterationResult CopyBootSlotsExperiment::Iteration()
        {
            // TODO: Get source boot slot based on majority voting of 3 source entries. Code below based on program_upload telecommands.
            int _selectedBootSlot = 0;
            auto&& source = this->_bootTable.Entry(_selectedBootSlot);
            uint16_t expectedCrc = source.CalculateCrc();

            UniqueLock<program_flash::BootTable> lock(this->_bootTable, InfiniteTimeout);

            for (auto i = 0; i < program_flash::BootTable::EntriesCount; i++)
            {
                if (_targetEntries[i])
                {
                    auto result = this->_bootTable.Entry(i).Erase();

                    if (!result)
                    {
                        // TODO: Are those messages needed as this is long running experiment and we can have no chance to recieve them?
                        _transmitter.SendFrame(EraseEntryError(num(std::get<0>(result.Error())), i, std::get<1>(result.Error())).Frame());
                        return IterationResult::Failure;
                    }

                    auto r = this->_bootTable.Entry(i).WriteContent(0, source.WholeEntry());

                    if (r != FlashStatus::NotBusy)
                    {
                        _transmitter.SendFrame(WriteProgramError(num(r), i, 0).Frame());
                        return IterationResult::Failure;
                    }

                    auto e = this->_bootTable.Entry(i);

                    r = FlashStatus::NotBusy;

                    r = Worst(r, e.Crc(expectedCrc));
                    // r = Worst(r, e.Length(length));
                    // r = Worst(r, e.Description(description.data()));
                    r = Worst(r, e.MarkAsValid());

                    if (r != program_flash::FlashStatus::NotBusy)
                    {
                        _transmitter.SendFrame(FinalizeEntryWriteError(num(r), i).Frame());
                        return IterationResult::Failure;
                    }

                    auto actualCrc = e.CalculateCrc();

                    if (actualCrc != expectedCrc)
                    {
                        _transmitter.SendFrame(FinalizeEntryCRCError(i, actualCrc).Frame());
                        return IterationResult::Failure;
                    }
                }
            }

            return IterationResult::Finished;
        }

        void CopyBootSlotsExperiment::Stop(IterationResult /*lastResult*/)
        {
        }
    }
}
