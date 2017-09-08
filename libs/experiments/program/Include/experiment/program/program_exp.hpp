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
             * @param fileName File name for output file
             *
             * @remark String is copied to internal buffer
             * @remark If string is longer than internal buffer size, it is trimmed to maximum size
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
             */
            CopyBootSlotsExperiment(program_flash::BootTable& bootTable, devices::comm::ITransmitter& transmitter);

            virtual experiments::ExperimentCode Type() override;
            virtual experiments::StartResult Start() override;
            virtual experiments::IterationResult Iteration() override;
            virtual void Stop(experiments::IterationResult lastResult) override;

            virtual void SetupEntries(BootEntriesSelector& source, BootEntriesSelector& target) override;

          private:
            /** @brief Boot Table */
            program_flash::BootTable& _bootTable;

            devices::comm::ITransmitter& _transmitter;

            BootEntriesSelector _sourceEntries;
            BootEntriesSelector _targetEntries;
        };
    }
}

#endif /* LIBS_EXPERIMENTS_PROGRAM_INCLUDE_EXPERIMENT_PROGRAM_PROGRAM_EXP_HPP_ */
