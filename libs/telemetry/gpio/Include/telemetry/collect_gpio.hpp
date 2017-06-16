#ifndef LIBS_TELEMETRY_GPIO_COLLECT_HPP
#define LIBS_TELEMETRY_GPIO_COLLECT_HPP

#pragma once

#include "antenna/antenna.h"
#include "gpio/gpio.h"
#include "mission/base.hpp"
#include "telemetry/state.hpp"

namespace telemetry
{
    /**
     * @brief This task is responsible for acquiring & updating telemetry via gpios
     * @telemetry_acquisition
     * @ingroup telemetry
     */
    template <typename SailDeployedPin> class GpioTelemetryAcquisition : public mission::Update
    {
      public:
        /**
         * @brief dummy
         * @param value ignored
         */
        GpioTelemetryAcquisition(int value);
        /**
         * @brief Builds update descriptor for this task.
         * @return Update descriptor - the antenna telemetry acquisition update task.
         */
        mission::UpdateDescriptor<telemetry::TelemetryState> BuildUpdate();

        /**
         * @brief Acquires antenna telemetry & stores it in passed state object.
         * @param[in] state Object that should be updated with new antenna hardware telemetry.
         * @return Telemetry acquisition result.
         */
        mission::UpdateResult UpdateTelemetry(telemetry::TelemetryState& state);

      private:
        /**
         * @brief Updates current gpio telemetry in global state.
         * @param[in] state Reference to global state.
         * @param[in] param Current execution context.
         */
        static mission::UpdateResult UpdateProc(telemetry::TelemetryState& state, void* param);

        /**
         * @brief Reference to antenna driver.
         */
        drivers::gpio::InputPin<SailDeployedPin> sailDeployedPin;
    };

    template <typename SailDeployedPin> GpioTelemetryAcquisition<SailDeployedPin>::GpioTelemetryAcquisition(int /*value*/)
    {
    }

    template <typename SailDeployedPin>
    mission::UpdateDescriptor<telemetry::TelemetryState> GpioTelemetryAcquisition<SailDeployedPin>::BuildUpdate()
    {
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
        descriptor.name = "GPIO Telemetry Acquisition";
        descriptor.updateProc = UpdateProc;
        descriptor.param = this;
        return descriptor;
    }

    template <typename SailDeployedPin>
    mission::UpdateResult GpioTelemetryAcquisition<SailDeployedPin>::UpdateTelemetry(telemetry::TelemetryState& state)
    {
        const auto sailSate = this->sailDeployedPin.Input();
        GpioState telemetry(sailSate);
        state.telemetry.Set(telemetry);
        return mission::UpdateResult::Ok;
    }

    template <typename SailDeployedPin>
    mission::UpdateResult GpioTelemetryAcquisition<SailDeployedPin>::UpdateProc(telemetry::TelemetryState& state, void* param)
    {
        auto This = static_cast<GpioTelemetryAcquisition<SailDeployedPin>*>(param);
        return This->UpdateTelemetry(state);
    }
}

#endif
