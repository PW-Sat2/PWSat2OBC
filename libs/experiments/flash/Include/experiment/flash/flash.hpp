#ifndef LIBS_EXPERIMENTS_FLASH_INCLUDE_EXPERIMENT_FLASH_HPP_
#define LIBS_EXPERIMENTS_FLASH_INCLUDE_EXPERIMENT_FLASH_HPP_

#include "comm/comm.hpp"
#include "experiments/experiments.h"
#include "n25q/fwd.hpp"

namespace experiment
{
    namespace erase_flash
    {
        /**
         * @brief Interface for setting up erase flash experiment
         * @ingroup experiments
         */
        struct ISetCorrelationId
        {
            /**
             * @brief Sets correlation ID for finished frame
             * @param correlationId Correlation ID
             */
            virtual void SetCorrelationId(std::uint8_t correlationId) = 0;
        };

        /**
         * @brief Erase flash experiment
         * @ingroup experiments
         */
        class EraseFlashExperiment : public experiments::IExperiment, public ISetCorrelationId
        {
          public:
            /**
             * @brief Ctor
             * @param n25q N25Q driver
             * @param transmitter Transmitter
             */
            EraseFlashExperiment(devices::n25q::RedundantN25QDriver& n25q, devices::comm::ITransmitter& transmitter);

            virtual void SetCorrelationId(std::uint8_t correlationId) override;

            virtual experiments::ExperimentCode Type() override;
            virtual experiments::StartResult Start() override;
            virtual experiments::IterationResult Iteration() override;
            virtual void Stop(experiments::IterationResult lastResult) override;

            /** @brief Experiment code */
            static constexpr experiments::ExperimentCode Code = 0x2;

          private:
            devices::n25q::RedundantN25QDriver& _n25q;
            devices::comm::ITransmitter& _transmitter;

            std::uint8_t _correlationId;
        };

        inline void EraseFlashExperiment::SetCorrelationId(std::uint8_t correlationId)
        {
            this->_correlationId = correlationId;
        }
    }
}

#endif /* LIBS_EXPERIMENTS_FLASH_INCLUDE_EXPERIMENT_FLASH_HPP_ */
