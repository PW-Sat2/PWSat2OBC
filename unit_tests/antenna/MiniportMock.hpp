#ifndef ANTENNA_MINIPORT_MOCK_HPP
#define ANTENNA_MINIPORT_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "antenna/miniport.h"

struct AntennaMiniportMock : AntennaMiniportDriver
{
    AntennaMiniportMock();

    MOCK_METHOD0(Reset, OSResult());

    MOCK_METHOD0(ArmDeploymentSystem, OSResult());

    MOCK_METHOD0(DisarmDeploymentSystem, OSResult());

    MOCK_METHOD0(InitializeAutomaticDeployment, OSResult());

    MOCK_METHOD0(CancelAntennaDeployment, OSResult());

    MOCK_METHOD2(DeployAntenna, OSResult(AntennaId antennaId, TimeSpan timeout));

    MOCK_METHOD2(DeployAntennaOverride, OSResult(AntennaId antennaId, TimeSpan timeout));

    MOCK_METHOD1(GetDeploymentStatus, OSResult(AntennaMiniportDeploymentStatus* telemetry));

    MOCK_METHOD2(GetAntennaActivationCount, OSResult(AntennaId antennaId, uint16_t* count));

    MOCK_METHOD2(GetAntennaActivationTime, OSResult(AntennaId antennaId, TimeSpan* span));

    MOCK_METHOD1(GetTemperature, OSResult(uint16_t* temperature));
};

#endif
