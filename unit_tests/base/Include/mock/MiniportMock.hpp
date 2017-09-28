#ifndef ANTENNA_MINIPORT_MOCK_HPP
#define ANTENNA_MINIPORT_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "antenna/miniport.h"

struct AntennaMiniportMock : AntennaMiniportDriver
{
    AntennaMiniportMock();
    ~AntennaMiniportMock();

    MOCK_METHOD2(Reset, OSResult(drivers::i2c::II2CBus* communicationBus, AntennaChannel channel));

    MOCK_METHOD2(ArmDeploymentSystem, OSResult(drivers::i2c::II2CBus* communicationBus, AntennaChannel channel));

    MOCK_METHOD2(DisarmDeploymentSystem, OSResult(drivers::i2c::II2CBus* communicationBus, AntennaChannel channel));

    MOCK_METHOD3(InitializeAutomaticDeployment,
        OSResult(drivers::i2c::II2CBus* communicationBus, AntennaChannel channel, std::chrono::milliseconds timeout));

    MOCK_METHOD2(CancelAntennaDeployment, OSResult(drivers::i2c::II2CBus* communicationBus, AntennaChannel channel));

    MOCK_METHOD5(DeployAntenna,
        OSResult(drivers::i2c::II2CBus* communicationBus,
            AntennaChannel channel,
            AntennaId antennaId,
            std::chrono::milliseconds timeout,
            bool override //
            ));

    MOCK_METHOD3(GetDeploymentStatus,
        OSResult(drivers::i2c::II2CBus* communicationBus,
            AntennaChannel channel,
            AntennaDeploymentStatus* telemetry //
            ));

    MOCK_METHOD4(GetAntennaActivationCount,
        OSResult(drivers::i2c::II2CBus* communicationBus,
            AntennaChannel channel,
            AntennaId antennaId,
            uint8_t* count //
            ));

    MOCK_METHOD4(GetAntennaActivationTime,
        OSResult(drivers::i2c::II2CBus* communicationBus,
            AntennaChannel channel,
            AntennaId antennaId,
            std::chrono::milliseconds* span) //
        );

    MOCK_METHOD3(GetTemperature, OSResult(drivers::i2c::II2CBus* communicationBus, AntennaChannel channel, uint16_t* temperature));
};

#endif
