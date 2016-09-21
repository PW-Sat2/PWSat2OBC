#ifndef SRC_DEVICES_ANTENNA_DRIVER_H_
#define SRC_DEVICES_ANTENNA_DRIVER_H_

#pragma once

#include "antenna.h"
#include "base/os.h"
#include "time/TimePoint.h"

enum AntenaPortStatus
{
    ANTENNA_PORT_OPERATIONAL,
    ANTENNA_PORT_FAILURE,
};

enum TelemetryField
{
    ANT_TM_ANTENNA_DEPLOYMENT_STATUS = 1 << 0,
    ANT_TM_ANTENNA_DEPLOYMENT_ACTIVE = 1 << 1,
    ANT_TM_ANTENNA1_ACTIVATION_COUNT = 1 << 2,
    ANT_TM_ANTENNA2_ACTIVATION_COUNT = 1 << 3,
    ANT_TM_ANTENNA3_ACTIVATION_COUNT = 1 << 4,
    ANT_TM_ANTENNA4_ACTIVATION_COUNT = 1 << 5,
    ANT_TM_ANTENNA1_ACTIVATION_TIME = 1 << 6,
    ANT_TM_ANTENNA2_ACTIVATION_TIME = 1 << 7,
    ANT_TM_ANTENNA3_ACTIVATION_TIME = 1 << 8,
    ANT_TM_ANTENNA4_ACTIVATION_TIME = 1 << 9,

    ANT_TM_TEMPERATURE1 = 1 << 10,
    ANT_TM_TEMPERATURE2 = 1 << 11,
    ANT_TM_SWITCHES_IGNORED1 = 1 << 12,
    ANT_TM_SWITCHES_IGNORED2 = 1 << 13,
    ANT_TM_DEPLOYMENT_SYSTEM_STATUS1 = 1 << 14,
    ANT_TM_DEPLOYMENT_SYSTEM_STATUS2 = 1 << 15,
};

struct AntennaTelemetry
{
    bool DeploymentStatus[4];
    bool IsDeploymentActive[4];
    uint8_t ActivationCount[4];
    TimeSpan ActivationTime[4];
    uint16_t Temperature[2];
    bool IgnoringDeploymentSwitches[2];
    bool DeploymentSystemArmed[2];
    uint32_t flags;
};

struct AntennaChannelInfo
{
    AntennaMiniportDriver* port;
    AntenaPortStatus status;
};

struct AntennaDriver
{
    AntennaChannelInfo primaryChannel;
    AntennaChannelInfo secondaryChannel;

    OSResult (*Reset)(struct AntennaDriver* driver);

    OSResult (*ArmDeploymentSystem)(struct AntennaDriver* driver);

    OSResult (*DisarmDeploymentSystem)(struct AntennaDriver* driver);

    OSResult (*DeployAntenna)(struct AntennaDriver* driver, AntennaId antennaId, TimeSpan timeout);

    OSResult (*DeployAntennaOverride)(struct AntennaDriver* driver, AntennaId antennaId, TimeSpan timeout);

    OSResult (*InitializeAutomaticDeployment)(struct AntennaDriver* driver);

    OSResult (*CancelAntennaDeployment)(struct AntennaDriver* driver);

    OSResult (*GetTemperature)(struct AntennaDriver* driver, uint16_t* temperature);

    AntennaTelemetry (*GetTelemetry)(struct AntennaDriver* driver);
};

void AntennaDriverInitialize(AntennaDriver* driver,
    AntennaMiniportDriver* primary,
    AntennaMiniportDriver* secondary //
    );

#endif
