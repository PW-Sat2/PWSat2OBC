#ifndef SRC_DEVICES_ANTENNA_MINIPORT_H_
#define SRC_DEVICES_ANTENNA_MINIPORT_H_

#pragma once

#include "antenna.h"
#include "base/os.h"
#include "i2c/i2c.h"
#include "time/TimePoint.h"

struct AntennaMiniportDeploymentStatus
{
    bool DeploymentStatus[4];
    bool IsDeploymentActive[4];
    bool IgnoringDeploymentSwitches;
    bool DeploymentSystemArmed;
};

struct AntennaMiniportDriver
{
    I2CBus* communicationBus;

    AntennaChannel currentChannel;

    OSResult (*Reset)(struct AntennaMiniportDriver* driver);

    OSResult (*ArmDeploymentSystem)(struct AntennaMiniportDriver* driver);

    OSResult (*DisarmDeploymentSystem)(struct AntennaMiniportDriver* driver);

    OSResult (*DeployAntenna)(struct AntennaMiniportDriver* driver, AntennaId antennaId, TimeSpan timeout);

    OSResult (*DeployAntennaOverride)(struct AntennaMiniportDriver* driver, AntennaId antennaId, TimeSpan timeout);

    OSResult (*InitializeAutomaticDeployment)(struct AntennaMiniportDriver* driver);

    OSResult (*CancelAntennaDeployment)(struct AntennaMiniportDriver* driver);

    OSResult (*GetDeploymentStatus)(struct AntennaMiniportDriver* driver, AntennaMiniportDeploymentStatus* telemetry);

    OSResult (*GetAntennaActivationCount)(struct AntennaMiniportDriver* driver, AntennaId antennaId, uint16_t* count);

    OSResult (*GetAntennaActivationTime)(struct AntennaMiniportDriver* driver, AntennaId antennaId, TimeSpan* count);

    OSResult (*GetTemperature)(struct AntennaMiniportDriver* driver, uint16_t* temperature);
};

void AntennaMiniportInitialize(AntennaMiniportDriver* driver,
    AntennaChannel currentChannel,
    I2CBus* dedicatedBus //
    );

#endif
