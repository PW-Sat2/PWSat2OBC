#ifndef ANTENNA_MINIPORT_MOCK_HPP
#define ANTENNA_MINIPORT_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "antenna/miniport.h"

struct AntennaMiniportMock : AntennaMiniportDriver
{
    AntennaMiniportMock();
    ~AntennaMiniportMock();

    MOCK_METHOD1(Reset, OSResult(AntennaChannel channel));

    MOCK_METHOD1(ArmDeploymentSystem, OSResult(AntennaChannel channel));

    MOCK_METHOD1(DisarmDeploymentSystem, OSResult(AntennaChannel channel));

    MOCK_METHOD2(InitializeAutomaticDeployment, OSResult(AntennaChannel channel, std::chrono::milliseconds timeout));

    MOCK_METHOD1(CancelAntennaDeployment, OSResult(AntennaChannel channel));

    MOCK_METHOD4(DeployAntenna,
        OSResult(AntennaChannel channel,
            AntennaId antennaId,
            std::chrono::milliseconds timeout,
            bool override //
            ));

    MOCK_METHOD2(GetDeploymentStatus,
        OSResult(AntennaChannel channel,
            AntennaDeploymentStatus* telemetry //
            ));

    MOCK_METHOD3(GetAntennaActivationCount,
        OSResult(AntennaChannel channel,
            AntennaId antennaId,
            uint8_t* count //
            ));

    MOCK_METHOD3(GetAntennaActivationTime,
        OSResult(AntennaChannel channel,
            AntennaId antennaId,
            std::chrono::milliseconds* span) //
        );

    MOCK_METHOD2(GetTemperature, OSResult(AntennaChannel channel, uint16_t* temperature));
};

#endif
