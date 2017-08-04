#ifndef LIBS_TELEMETRY_FWD_HPP
#define LIBS_TELEMETRY_FWD_HPP

#pragma once

#include "antenna/antenna.h"
#include "comm/comm.hpp"
#include "eps/eps.h"
#include "gyro/gyro.h"
#include "imtq/fwd.hpp"
#include "state/fwd.hpp"

namespace telemetry
{
    class SystemStartup;
    class ErrorCountingTelemetry;
    class ExperimentTelemetry;
    class InternalTimeTelemetry;
    class ExternalTimeTelemetry;
    class ImtqHousekeeping;
    class ImtqState;

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
        struct MagnetometerMeasurementsTag;
        struct DipolesTag;
        struct BDotTag;
        struct CoilCurrentTag;
        struct CoilTemperatureTag;
        struct CoilsActiveTag;
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

    /**
     * @brief This type represents telemetry element related to imtq magnetometer measurements
     * @telemetry_element
     * @ingroup telemetry
     */
    typedef SimpleTelemetryElement<std::array<devices::imtq::MagnetometerMeasurement, 3>, ::telemetry::details::MagnetometerMeasurementsTag>
        ImtqMagnetometerMeasurements;

    /**
     * @brief This type represents telemetry element related to imtq observed dipoles
     * @telemetry_element
     * @ingroup telemetry
     */
    typedef SimpleTelemetryElement<std::array<devices::imtq::Dipole, 3>, ::telemetry::details::DipolesTag> ImtqDipoles;

    /**
     * @brief This type represents telemetry element related to imtq b-dot field
     * @telemetry_element
     * @ingroup telemetry
     */
    typedef SimpleTelemetryElement<std::array<devices::imtq::BDotType, 3>, ::telemetry::details::BDotTag> ImtqBDotTelemetry;

    /**
     * @brief This type represents telemetry element related to imtq self test result
     * @telemetry_element
     * @ingroup telemetry
     */
    typedef SimpleTelemetryElement<std::array<std::uint8_t, 8>, ::telemetry::details::BDotTag> ImtqSelfTest;

    /**
     * @brief This type represents telemetry element related to imtq coils state during magnetometer measurements
     * @telemetry_element
     * @ingroup telemetry
     */
    typedef SimpleTelemetryElement<bool, ::telemetry::details::CoilsActiveTag> ImtqCoilsActive;

    using ImtqTelemetryCurrent = devices::imtq::Current;

    /**
     * @brief This type represents telemetry element related to imtq coils' currents
     * @telemetry_element
     * @ingroup telemetry
     */
    typedef SimpleTelemetryElement<std::array<ImtqTelemetryCurrent, 3>, ::telemetry::details::CoilCurrentTag> ImtqCoilCurrent;

    using ImtqTelemetryTemperature = devices::imtq::TemperatureMeasurement;

    /**
     * @brief This type represents telemetry element related to imtq coils' temperatures
     * @telemetry_element
     * @ingroup telemetry
     */
    typedef SimpleTelemetryElement<std::array<ImtqTelemetryTemperature, 3>, ::telemetry::details::CoilTemperatureTag> ImtqCoilTemperature;

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
        devices::eps::hk::ControllerBTelemetry, //
        ImtqMagnetometerMeasurements,           //
        ImtqCoilsActive,                        //
        ImtqDipoles,                            //
        ImtqBDotTelemetry,                      //
        ImtqHousekeeping,                       //
        ImtqCoilCurrent,                        //
        ImtqCoilTemperature,                    //
        ImtqState,                              //
        ImtqSelfTest                            //
        >
        ManagedTelemetry;
}

#endif
