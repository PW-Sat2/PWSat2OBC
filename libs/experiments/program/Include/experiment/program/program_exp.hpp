#ifndef LIBS_EXPERIMENTS_PROGRAM_INCLUDE_EXPERIMENT_PROGRAM_PROGRAM_EXP_HPP_
#define LIBS_EXPERIMENTS_PROGRAM_INCLUDE_EXPERIMENT_PROGRAM_PROGRAM_EXP_HPP_

#include <bitset>
#include <chrono>
#include "comm/comm.hpp"
#include "experiments/experiments.h"
#include "program_flash/boot_table.hpp"

namespace experiment
{
    namespace program
    {
        using BootEntriesSelector = std::bitset<program_flash::BootTable::EntriesCount>;

        /**
         * @brief Interface for setting-up Copy Boot Slots experiment parameters
         */
        struct ISetupCopyBootSlotsExperiment
        {
            /**
             * @brief Sets file name for output file
             * @param source Bit mask for three source slots
             * @param target Bit mask target slots
             */
            virtual void SetupEntries(BootEntriesSelector& source, BootEntriesSelector& target) = 0;
        };

        /**
         * @brief Copy Boot Slots "experiment"
         * @ingroup experiments
         */
        class CopyBootSlotsExperiment final : public experiments::IExperiment, public ISetupCopyBootSlotsExperiment
        {
          public:
            /** @brief Experiment code */
            static constexpr experiments::ExperimentCode Code = 11;

            /**
             * @brief Ctor
             * @param bootTable The boot table
             * @param flashDriver Program flash driver
             * @param transmitter Transmitter
             */
            CopyBootSlotsExperiment(
                program_flash::BootTable& bootTable, program_flash::IFlashDriver& flashDriver, devices::comm::ITransmitter& transmitter);

            virtual experiments::ExperimentCode Type() override;
            virtual experiments::StartResult Start() override;
            virtual experiments::IterationResult Iteration() override;
            virtual void Stop(experiments::IterationResult lastResult) override;

            virtual void SetupEntries(BootEntriesSelector& source, BootEntriesSelector& target) override;

          private:
            /** @brief Boot Table */
            program_flash::BootTable& _bootTable;
            program_flash::IFlashDriver& _flashDriver;

            devices::comm::ITransmitter& _transmitter;

            BootEntriesSelector _sourceEntries;
            BootEntriesSelector _targetEntries;
        };
    }
}

#endif /* LIBS_EXPERIMENTS_PROGRAM_INCLUDE_EXPERIMENT_PROGRAM_PROGRAM_EXP_HPP_ */
