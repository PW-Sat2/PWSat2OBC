#ifndef ANTENNA_MOCK_HPP
#define ANTENNA_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "antenna/driver.h"

struct AntennaMock : public AntennaDriver
{
    AntennaMock();

    MOCK_METHOD1(Reset, OSResult(AntennaChannel channel));

    MOCK_METHOD0(HardReset, OSResult());

    MOCK_METHOD1(FinishDeployment, OSResult(AntennaChannel channel));

    MOCK_METHOD4(DeployAntenna,
        OSResult(AntennaChannel channel,
            AntennaId antennaId,
            TimeSpan timeout,
            bool overrideSwitches //
            ));

    MOCK_METHOD2(GetDeploymentStatus, OSResult(AntennaChannel channel, AntennaDeploymentStatus* telemetry));

    MOCK_METHOD2(GetTemperature, OSResult(AntennaChannel channel, uint16_t* temperature));

    MOCK_METHOD0(GetTelemetry, AntennaTelemetry());
};

#endif
