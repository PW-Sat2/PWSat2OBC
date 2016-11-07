#ifndef ANTENNA_MINIPORT_MOCK_HPP
#define ANTENNA_MINIPORT_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "antenna/miniport.h"

struct AntennaMiniportMock : AntennaMiniportDriver
{
    AntennaMiniportMock();

    MOCK_METHOD2(Reset,
        OSResult(I2CBus* communicationBus,
                     AntennaChannel channel) //
        );

    MOCK_METHOD2(ArmDeploymentSystem,
        OSResult(I2CBus* communicationBus,
                     AntennaChannel channel) //
        );

    MOCK_METHOD2(DisarmDeploymentSystem,
        OSResult(I2CBus* communicationBus,
                     AntennaChannel channel) //
        );

    MOCK_METHOD2(InitializeAutomaticDeployment,
        OSResult(I2CBus* communicationBus,
                     AntennaChannel channel) //
        );

    MOCK_METHOD2(CancelAntennaDeployment,
        OSResult(I2CBus* communicationBus,
                     AntennaChannel channel) //
        );

    MOCK_METHOD5(DeployAntenna,
        OSResult(I2CBus* communicationBus,
                     AntennaChannel channel,
                     AntennaId antennaId,
                     TimeSpan timeout,
                     bool override) //
        );

    MOCK_METHOD3(GetDeploymentStatus,
        OSResult(I2CBus* communicationBus,
                     AntennaChannel channel,
                     AntennaDeploymentStatus* telemetry) //
        );

    MOCK_METHOD4(GetAntennaActivationCount,
        OSResult(I2CBus* communicationBus,
                     AntennaChannel channel,
                     AntennaId antennaId,
                     uint16_t* count) //
        );

    MOCK_METHOD4(GetAntennaActivationTime,
        OSResult(I2CBus* communicationBus,
                     AntennaChannel channel,
                     AntennaId antennaId,
                     TimeSpan* span) //
        );

    MOCK_METHOD3(GetTemperature,
        OSResult(I2CBus* communicationBus,
                     AntennaChannel channel,
                     uint16_t* temperature) //
        );
};

#endif
