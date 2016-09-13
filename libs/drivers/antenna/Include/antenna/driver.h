#ifndef SRC_DEVICES_ANTENNA_DRIVER_H_
#define SRC_DEVICES_ANTENNA_DRIVER_H_

#pragma once

#include "antenna.h"
#include "system.h"

EXTERNC_BEGIN

typedef enum {
    ANTENNA_PORT_OPERATIONAL,
    ANTENNA_PORT_FAILURE,
} AntenaPortStatus;

typedef struct
{
    bool DeploymentStatus[4];
    bool IsDeploymentActive[4];
    uint8_t ActivationCount[4];
    uint32_t ActivationTime[4];
    uint16_t Temperature;
    bool IgnoringDeploymentSwitches;
    bool DeploymentSystemArmed;
} AntennaTelemetry;

typedef struct AntennaDriver
{
    AntennaMiniportDriver* primaryPort;
    AntennaMiniportDriver* secondaryPort;
    AntenaPortStatus primaryPortStatus;
    AntenaPortStatus secondaryPortStatus;

    OSResult (*InitializeDeployment)(struct AntennaDriver* driver);

    OSResult (*Reset)(struct AntennaDriver* driver);

    OSResult (*ArmDeploymentSystem)(struct AntennaDriver* driver);

    OSResult (*DisarmDeploymentSystem)(struct AntennaDriver* driver);

    OSResult (*DeployAntenna)(struct AntennaDriver* driver, AntennaId antennaId, TimeSpan timeout);

    OSResult (*DeployAntennaOverride)(struct AntennaDriver* driver, AntennaId antennaId, TimeSpan timeout);

    OSResult (*InitializeAutomaticDeployment)(struct AntennaDriver* driver);

    OSResult (*GetTelemetry)(struct AntennaDriver* driver, AntennaTelemetry* telemetry);

} AntennaDriver;

OSResult AntennaDriverInitialize(AntennaDriver* driver,
    AntennaMiniportDriver* primary,
    AntennaMiniportDriver* secondary //
    );

EXTERNC_END

#endif
