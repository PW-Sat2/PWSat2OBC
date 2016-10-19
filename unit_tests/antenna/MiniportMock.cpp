#include "MiniportMock.hpp"

static OSResult Reset(struct AntennaMiniportDriver* driver)
{
    auto mock = static_cast<AntennaMiniportMock*>(driver);
    return mock->Reset();
}

static OSResult ArmDeploymentSystem(struct AntennaMiniportDriver* driver)
{
    auto mock = static_cast<AntennaMiniportMock*>(driver);
    return mock->ArmDeploymentSystem();
}

static OSResult DisarmDeploymentSystem(struct AntennaMiniportDriver* driver)
{
    auto mock = static_cast<AntennaMiniportMock*>(driver);
    return mock->DisarmDeploymentSystem();
}

static OSResult DeployAntenna(struct AntennaMiniportDriver* driver, AntennaId antennaId, TimeSpan timeout)
{
    auto mock = static_cast<AntennaMiniportMock*>(driver);
    return mock->DeployAntenna(antennaId, timeout);
}

static OSResult DeployAntennaOverride(struct AntennaMiniportDriver* driver, AntennaId antennaId, TimeSpan timeout)
{
    auto mock = static_cast<AntennaMiniportMock*>(driver);
    return mock->DeployAntennaOverride(antennaId, timeout);
}

static OSResult InitializeAutomaticDeployment(struct AntennaMiniportDriver* driver)
{
    auto mock = static_cast<AntennaMiniportMock*>(driver);
    return mock->InitializeAutomaticDeployment();
}

static OSResult CancelAntennaDeployment(struct AntennaMiniportDriver* driver)
{
    auto mock = static_cast<AntennaMiniportMock*>(driver);
    return mock->CancelAntennaDeployment();
}

static OSResult GetDeploymentStatus(struct AntennaMiniportDriver* driver, AntennaMiniportDeploymentStatus* telemetry)
{
    auto mock = static_cast<AntennaMiniportMock*>(driver);
    return mock->GetDeploymentStatus(telemetry);
}

static OSResult GetAntennaActivationCount(struct AntennaMiniportDriver* driver, AntennaId antennaId, uint16_t* count)
{
    auto mock = static_cast<AntennaMiniportMock*>(driver);
    return mock->GetAntennaActivationCount(antennaId, count);
}

static OSResult GetAntennaActivationTime(struct AntennaMiniportDriver* driver, AntennaId antennaId, TimeSpan* count)
{
    auto mock = static_cast<AntennaMiniportMock*>(driver);
    return mock->GetAntennaActivationTime(antennaId, count);
}

static OSResult GetTemperature(struct AntennaMiniportDriver* driver, uint16_t* temperature)
{
    auto mock = static_cast<AntennaMiniportMock*>(driver);
    return mock->GetTemperature(temperature);
}

AntennaMiniportMock::AntennaMiniportMock()
{
    auto This = static_cast<AntennaMiniportDriver*>(this);
    This->Reset = ::Reset;
    This->ArmDeploymentSystem = ::ArmDeploymentSystem;
    This->DisarmDeploymentSystem = ::DisarmDeploymentSystem;
    This->DeployAntenna = ::DeployAntenna;
    This->DeployAntennaOverride = ::DeployAntennaOverride;
    This->InitializeAutomaticDeployment = ::InitializeAutomaticDeployment;
    This->CancelAntennaDeployment = ::CancelAntennaDeployment;
    This->GetDeploymentStatus = ::GetDeploymentStatus;
    This->GetAntennaActivationCount = ::GetAntennaActivationCount;
    This->GetAntennaActivationTime = ::GetAntennaActivationTime;
    This->GetTemperature = ::GetTemperature;
}
