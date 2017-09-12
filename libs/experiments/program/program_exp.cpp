#include "program_exp.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <gsl/span>
#include "base/os.h"
#include "base/writer.h"
#include "comm/ITransmitter.hpp"
#include "logger/logger.h"
#include "redundancy.hpp"
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

        CopyBootSlotsExperiment::CopyBootSlotsExperiment(
            program_flash::BootTable& bootTable, program_flash::IFlashDriver& flashDriver, devices::comm::ITransmitter& transmitter)
            : _bootTable(bootTable), _flashDriver(flashDriver), _transmitter(transmitter)
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

        static std::array<std::uint8_t, 3> ToSlotNumbers(BootEntriesSelector mask)
        {
            std::array<std::uint8_t, 3> slots{0, 0, 0};

            for (auto i = 0, j = 0; i < 6; i++)
            {
                if (mask[i])
                {
                    slots[j] = i;
                    j++;
                }

                if (j == 3)
                    break;
            }

            return slots;
        }

        IterationResult CopyBootSlotsExperiment::Iteration()
        {
            auto slots = ToSlotNumbers(_sourceEntries);

            std::array<gsl::span<const std::uint8_t>, 3> entryBases;
            std::transform(
                slots.begin(), slots.end(), entryBases.begin(), [this](auto& e) { return this->_bootTable.Entry(e).WholeEntry(); });

            UniqueLock<program_flash::BootTable> lock(this->_bootTable, InfiniteTimeout);

            for (auto i = 0; i < program_flash::BootTable::EntriesCount; i++)
            {
                if (_targetEntries[i])
                {
                    auto target = this->_bootTable.Entry(i);
                    auto result = target.Erase();

                    if (!result)
                    {
                        _transmitter.SendFrame(EraseEntryError(num(std::get<0>(result.Error())), i, std::get<1>(result.Error())).Frame());
                        return IterationResult::Failure;
                    }
                }
            }

            std::array<std::uint8_t, 1_KB> copyBuffer;

            for (std::size_t offset = 0; offset < program_flash::ProgramEntry::Size; offset += copyBuffer.size())
            {
                auto partSize = std::min(copyBuffer.size(), program_flash::ProgramEntry::Size - offset);

                std::array<gsl::span<const std::uint8_t>, 3> parts;
                std::transform(entryBases.begin(), entryBases.end(), parts.begin(), [offset, partSize](auto& p) {
                    return p.subspan(offset, partSize);
                });

                redundancy::CorrectBuffer(copyBuffer, parts[0], parts[1], parts[2]);

                for (auto i = 0; i < program_flash::BootTable::EntriesCount; i++)
                {
                    if (_targetEntries[i])
                    {
                        auto target = this->_bootTable.Entry(i);
                        this->_flashDriver.Program(target.InFlashOffset() + offset, copyBuffer);
                    }
                }
            }

            DownlinkFrame response(DownlinkAPID::CopyBootTable, 0);
            response.PayloadWriter().WriteByte(0);

            _transmitter.SendFrame(response.Frame());

            return IterationResult::Finished;
        }

        void CopyBootSlotsExperiment::Stop(IterationResult /*lastResult*/)
        {
        }
    }
}
