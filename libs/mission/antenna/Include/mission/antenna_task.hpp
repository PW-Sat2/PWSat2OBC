#ifndef LIBS_MISSION_ANTENNA_TASK_HPP
#define LIBS_MISSION_ANTENNA_TASK_HPP

#pragma once

#include <array>
#include <atomic>
#include <tuple>
#include "antenna/fwd.hpp"
#include "antenna/telemetry.hpp"
#include "mission/base.hpp"
#include "power/fwd.hpp"
#include "state/struct.h"

namespace mission
{
    namespace antenna
    {
        /**
         * @brief Mission part related to antenna deployment.
         * @ingroup mission_atenna
         * @mission_task
         *
         * This components provides two mission fragments related to antenna deployment: Action & Update parts.
         *
         * The purpose of the Update part is to keep updating the global satellite state with current antenna
         * deployment status.
         *
         * The purpose of Action part is to coordinate the antenna deployment. This part is executed
         * only if following condition are met:
         * - The initial silent mission period is over
         * - The antennas are not currently being deployed
         */
        class AntennaTask : public Update, public Action, public devices::antenna::IAntennaTelemetryProvider
        {
          public:
            /**
             * @brief ctor.
             * @param[in] args Tuple of: reference to antenna driver interface and reference to power control interface
             */
            AntennaTask(std::tuple<IAntennaDriver&, services::power::IPowerControl&> args);

            /**
             * @brief Initializes antenna mission state
             * @return Operation result
             */
            bool Initialize();

            /**
             * @brief Returns antenna deployment action descriptor.
             *
             * @returns Action descriptor that runs antenna deployment process.
             */
            ActionDescriptor<SystemState> BuildAction();

            /**
             * @brief Returns antenna deployment update descriptor.
             *
             * @returns Update descriptor that runs antenna deployment update process.
             */
            UpdateDescriptor<SystemState> BuildUpdate();

            virtual bool GetTelemetry(devices::antenna::AntennaTelemetry& telemetry) const override;

            /**
             * @brief Powers on selected power channel
             * @param task Pointer to antenna task
             * @param channel Channel to power on
             * @param antenna Not used
             * @param burnTime Not used
             * @return Operation result
             */
            static OSResult PowerOn(AntennaTask* task, AntennaChannel channel, AntennaId antenna, std::chrono::milliseconds burnTime);

            /**
             * @brief Resets selected controller
             * @param task Pointer to antenna task
             * @param channel Controller to reset
             * @param antenna Not used
             * @param burnTime Not used
             * @return Operation result
             */
            static OSResult Reset(AntennaTask* task, AntennaChannel channel, AntennaId antenna, std::chrono::milliseconds burnTime);

            /**
             * @brief Arms selected controller
             * @param task Pointer to antenna task
             * @param channel Controller to arm
             * @param antenna Not used
             * @param burnTime Not used
             * @return Operation result
             */
            static OSResult Arm(AntennaTask* task, AntennaChannel channel, AntennaId antenna, std::chrono::milliseconds burnTime);

            /**
             * @brief Performs deployment of selected antenna on selected controller
             * @param task Pointer to antenna task
             * @param channel Controller to for deploymeny
             * @param antenna Antenna to deploy
             * @param burnTime Burn time
             * @return Operation result
             */
            static OSResult Deploy(AntennaTask* task, AntennaChannel channel, AntennaId antenna, std::chrono::milliseconds burnTime);

            /**
             * @brief Disarms selected controller
             * @param task Pointer to antenna task
             * @param channel Controller to disarm
             * @param antenna Not used
             * @param burnTime Not used
             * @return Operation result
             */
            static OSResult Disarm(AntennaTask* task, AntennaChannel channel, AntennaId antenna, std::chrono::milliseconds burnTime);

            /**
             * @brief Powers off selected power channel
             * @param task Pointer to antenna task
             * @param channel Channel to power off
             * @param antenna Not used
             * @param burnTime Not used
             * @return Operation result
             */
            static OSResult PowerOff(AntennaTask* task, AntennaChannel channel, AntennaId antenna, std::chrono::milliseconds burnTime);

          private:
            /**
             * @brief Deploy condition
             * @param state System state
             * @param param Pointer to antenna task
             * @return true of action should be performed, false otherwise
             */
            static bool Condition(const SystemState& state, void* param);
            /**
             * @brief Performs single deployment step
             * @param state System state
             * @param param Pointer to antenna task
             */
            static void Action(SystemState& state, void* param);

            /**
             * @brief Collects antenna telemetry
             * @param state System state
             * @param param Pointer to antenna task
             * @return Operation result
             */
            static UpdateResult Update(SystemState& state, void* param);

            bool IsDeploymentDisabled(const SystemState& state);

            /**
             * @brief Type of function performing single deployment step
             */
            using StepAction = OSResult (*)(
                AntennaTask* task, AntennaChannel channel, AntennaId antenna, std::chrono::milliseconds burnTime);

            /**
             * @brief Single step descriptor
             */
            struct StepDescriptor
            {
                /** @brief Pointer to function to run */
                StepAction Action;
                /** @brief Controller to use */
                AntennaChannel Channel;
                /** @brief Antenna to use */
                AntennaId Antenna;
                /** @brief Burn time for deployment */
                std::chrono::milliseconds burnTime;
                /** @brief Wait time before next step */
                std::chrono::milliseconds waitTime;
            };

            /** @brief Array fo all steps */
            static std::array<StepDescriptor, 60> Steps;

            /** @brief Maximum number of retries per step */
            static constexpr std::int8_t StepRetries = 3;

            /** @brief Power controler interface */
            services::power::IPowerControl& _powerControl;
            /** @brief Antenna driver */
            IAntennaDriver& _antenna;

            /** @brief Current step */
            std::uint16_t _step;
            /** @brief Timestemp at which next step should be performed */
            std::chrono::milliseconds _nextStepAt;
            /** @brief Retry counter */
            std::int8_t _retryCounter;

            /** @brief Current antenna telemetry */
            devices::antenna::AntennaTelemetry _currentTelemetry;

            /** @brief Synchronization semaphore */
            OSSemaphoreHandle _sync;

            /** @brief Flag indicating whether controller is powered on */
            bool _controllerPoweredOn;
        };
    }
}

#endif
