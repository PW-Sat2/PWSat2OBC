#ifndef ANTENNA_MINIPORT_MOCK_HPP
#define ANTENNA_MINIPORT_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "antenna/miniport.h"

struct AntennaMiniportMock : AntennaMiniportDriver
{
    AntennaMiniportMock();

    MOCK_METHOD1(Reset, OSResult(AntennaChannel channel));

    MOCK_METHOD1(ArmDeploymentSystem, OSResult(AntennaChannel channel));

    MOCK_METHOD1(DisarmDeploymentSystem, OSResult(AntennaChannel channel));

    MOCK_METHOD2(InitializeAutomaticDeployment, OSResult(AntennaChannel channel, TimeSpan timeout));

    MOCK_METHOD1(CancelAntennaDeployment, OSResult(AntennaChannel channel));

    MOCK_METHOD4(DeployAntenna,
        OSResult(AntennaChannel channel,
                     AntennaId antennaId,
                     TimeSpan timeout,
                     bool override //
                     ));

    MOCK_METHOD2(GetDeploymentStatus,
        OSResult(AntennaChannel channel,
                     AntennaDeploymentStatus* telemetry //
                     ));

    MOCK_METHOD3(GetAntennaActivationCount,
        OSResult(AntennaChannel channel,
                     AntennaId antennaId,
                     uint16_t* count //
                     ));

    MOCK_METHOD3(GetAntennaActivationTime,
        OSResult(AntennaChannel channel,
                     AntennaId antennaId,
                     TimeSpan* span) //
        );

    MOCK_METHOD2(GetTemperature, OSResult(AntennaChannel channel, uint16_t* temperature));
};

#endif
