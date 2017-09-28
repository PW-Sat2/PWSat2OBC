#ifndef ANTENNA_MINIPORT_MOCK_HPP
#define ANTENNA_MINIPORT_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "antenna/miniport.h"

struct AntennaMiniportMock : AntennaMiniportDriver
{
    AntennaMiniportMock();
    ~AntennaMiniportMock();

    MOCK_METHOD3(
        Reset, OSResult(error_counter::AggregatedErrorCounter& error, drivers::i2c::II2CBus* communicationBus, AntennaChannel channel));

    MOCK_METHOD3(ArmDeploymentSystem,
        OSResult(error_counter::AggregatedErrorCounter& error, drivers::i2c::II2CBus* communicationBus, AntennaChannel channel));

    MOCK_METHOD3(DisarmDeploymentSystem,
        OSResult(error_counter::AggregatedErrorCounter& error, drivers::i2c::II2CBus* communicationBus, AntennaChannel channel));

    MOCK_METHOD4(InitializeAutomaticDeployment,
        OSResult(error_counter::AggregatedErrorCounter& error,
            drivers::i2c::II2CBus* communicationBus,
            AntennaChannel channel,
            std::chrono::milliseconds timeout));

    MOCK_METHOD3(CancelAntennaDeployment,
        OSResult(error_counter::AggregatedErrorCounter& error, drivers::i2c::II2CBus* communicationBus, AntennaChannel channel));

    MOCK_METHOD6(DeployAntenna,
        OSResult(error_counter::AggregatedErrorCounter& error,
            drivers::i2c::II2CBus* communicationBus,
            AntennaChannel channel,
            AntennaId antennaId,
            std::chrono::milliseconds timeout,
            bool override //
            ));

    MOCK_METHOD4(GetDeploymentStatus,
        OSResult(error_counter::AggregatedErrorCounter& error,
            drivers::i2c::II2CBus* communicationBus,
            AntennaChannel channel,
            AntennaDeploymentStatus* telemetry //
            ));

    MOCK_METHOD5(GetAntennaActivationCount,
        OSResult(error_counter::AggregatedErrorCounter& error,
            drivers::i2c::II2CBus* communicationBus,
            AntennaChannel channel,
            AntennaId antennaId,
            uint8_t* count //
            ));

    MOCK_METHOD5(GetAntennaActivationTime,
        OSResult(error_counter::AggregatedErrorCounter& error,
            drivers::i2c::II2CBus* communicationBus,
            AntennaChannel channel,
            AntennaId antennaId,
            std::chrono::milliseconds* span) //
        );

    MOCK_METHOD4(GetTemperature,
        OSResult(error_counter::AggregatedErrorCounter& error,
            drivers::i2c::II2CBus* communicationBus,
            AntennaChannel channel,
            uint16_t* temperature));
};

#endif
