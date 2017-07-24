#ifndef LIBS_TELEMETRY_FWD_HPP
#define LIBS_TELEMETRY_FWD_HPP

#pragma once

#include "antenna/antenna.h"
#include "comm/comm.hpp"
#include "eps/eps.h"
#include "gyro/gyro.h"
#include "state/fwd.hpp"

namespace telemetry
{
    class SystemStartup;
    class ErrorCountingTelemetry;
    class ExperimentTelemetry;
    class InternalTimeTelemetry;
    class ExternalTimeTelemetry;

    struct TelemetryState;

    template <typename T, typename Tag> class SimpleTelemetryElement;

    namespace details
    {
        struct FileSystemTelemetryTag;
        struct GpioStateTag;
        struct McuTemperatureTag;
        struct ProgramStateTag;
        struct FlashPrimarySlotsScrubbingTag;
        struct FlashSecondarySlotsScrubbingTag;
        struct RAMScrubbingTag;
    }

    /**
     * @brief This type represents telemetry element related file system state.
     * @telemetry_element
     * @ingroup telemetry
     */
    typedef SimpleTelemetryElement<std::uint32_t, ::telemetry::details::FileSystemTelemetryTag> FileSystemTelemetry;

    /**
     * @brief This class represents the state that is observed by the mcu via its gpios.
     * @telemetry_element
     */
    typedef SimpleTelemetryElement<bool, ::telemetry::details::GpioStateTag> GpioState;

    /**
     * @brief This type represents telemetry element related mcu temperature.
     * @telemetry_element
     * @ingroup telemetry
     */
    typedef SimpleTelemetryElement<BitValue<std::uint16_t, 12>, ::telemetry::details::McuTemperatureTag> McuTemperature;

    /**
     * @brief This type represents telemetry element related to
     * state of the currently executed program.
     * @telemetry_element
     * @ingroup telemetry
     */
    typedef SimpleTelemetryElement<std::uint16_t, ::telemetry::details::ProgramStateTag> ProgramState;

    /**
     * @brief This type represents telemetry element related to primary flash scrubber.
     * @telemetry_element
     * @ingroup telemetry
     */
    typedef SimpleTelemetryElement<BitValue<std::uint8_t, 3>, ::telemetry::details::FlashPrimarySlotsScrubbingTag>
        FlashPrimarySlotsScrubbing;

    /**
     * @brief This type represents telemetry element related to secondary flash scrubber.
     * @telemetry_element
     * @ingroup telemetry
     */
    typedef SimpleTelemetryElement<BitValue<std::uint8_t, 3>, ::telemetry::details::FlashSecondarySlotsScrubbingTag>
        FlashSecondarySlotsScrubbing;

    /**
     * @brief This type represents telemetry element related to RAM scrubber.
     * @telemetry_element
     * @ingroup telemetry
     */
    typedef SimpleTelemetryElement<std::uint32_t, ::telemetry::details::RAMScrubbingTag> RAMScrubbing;

    template <typename... Type> class Telemetry;

    typedef Telemetry<SystemStartup,            //
        ProgramState,                           //
        InternalTimeTelemetry,                  //
        ExternalTimeTelemetry,                  //
        ErrorCountingTelemetry,                 //
        FlashPrimarySlotsScrubbing,             //
        FlashSecondarySlotsScrubbing,           //
        RAMScrubbing,                           //
        FileSystemTelemetry,                    //
        devices::antenna::AntennaTelemetry,     //
        ExperimentTelemetry,                    //
        devices::gyro::GyroscopeTelemetry,      //
        devices::comm::CommTelemetry,           //
        GpioState,                              //
        McuTemperature,                         //
        devices::eps::hk::ControllerATelemetry, //
        devices::eps::hk::ControllerBTelemetry  //
        >
        ManagedTelemetry;
}

#endif
