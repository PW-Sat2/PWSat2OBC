#include "MiniportMock.hpp"

using drivers::i2c::II2CBus;

static OSResult Reset(struct AntennaMiniportDriver* driver,
    II2CBus* /*communicationBus*/,
    AntennaChannel channel //
    )
{
    auto mock = static_cast<AntennaMiniportMock*>(driver);
    return mock->Reset(channel);
}

static OSResult ArmDeploymentSystem(struct AntennaMiniportDriver* driver,
    II2CBus* /*communicationBus*/,
    AntennaChannel channel //
    )
{
    auto mock = static_cast<AntennaMiniportMock*>(driver);
    return mock->ArmDeploymentSystem(channel);
}

static OSResult DisarmDeploymentSystem(struct AntennaMiniportDriver* driver,
    II2CBus* /*communicationBus*/,
    AntennaChannel channel //
    )
{
    auto mock = static_cast<AntennaMiniportMock*>(driver);
    return mock->DisarmDeploymentSystem(channel);
}

static OSResult DeployAntenna(struct AntennaMiniportDriver* driver,
    II2CBus* /*communicationBus*/,
    AntennaChannel channel,
    AntennaId antennaId,
    std::chrono::milliseconds timeout,
    bool override //
    )
{
    auto mock = static_cast<AntennaMiniportMock*>(driver);
    return mock->DeployAntenna(channel, antennaId, timeout, override);
}

static OSResult InitializeAutomaticDeployment(struct AntennaMiniportDriver* driver,
    II2CBus* /*communicationBus*/,
    AntennaChannel channel,
    std::chrono::milliseconds timeout //
    )
{
    auto mock = static_cast<AntennaMiniportMock*>(driver);
    return mock->InitializeAutomaticDeployment(channel, timeout);
}

static OSResult CancelAntennaDeployment(struct AntennaMiniportDriver* driver,
    II2CBus* /*communicationBus*/,
    AntennaChannel channel //
    )
{
    auto mock = static_cast<AntennaMiniportMock*>(driver);
    return mock->CancelAntennaDeployment(channel);
}

static OSResult GetDeploymentStatus(struct AntennaMiniportDriver* driver,
    II2CBus* /*communicationBus*/,
    AntennaChannel channel,
    AntennaDeploymentStatus* telemetry //
    )
{
    auto mock = static_cast<AntennaMiniportMock*>(driver);
    return mock->GetDeploymentStatus(channel, telemetry);
}

static OSResult GetAntennaActivationCount(struct AntennaMiniportDriver* driver,
    II2CBus* /*communicationBus*/,
    AntennaChannel channel,
    AntennaId antennaId,
    uint8_t* count //
    )
{
    auto mock = static_cast<AntennaMiniportMock*>(driver);
    return mock->GetAntennaActivationCount(channel, antennaId, count);
}

static OSResult GetAntennaActivationTime(struct AntennaMiniportDriver* driver,
    II2CBus* /*communicationBus*/,
    AntennaChannel channel,
    AntennaId antennaId,
    std::chrono::milliseconds* count //
    )
{
    auto mock = static_cast<AntennaMiniportMock*>(driver);
    return mock->GetAntennaActivationTime(channel, antennaId, count);
}

static OSResult GetTemperature(struct AntennaMiniportDriver* driver,
    II2CBus* /*communicationBus*/,
    AntennaChannel channel,
    uint16_t* temperature //
    )
{
    auto mock = static_cast<AntennaMiniportMock*>(driver);
    return mock->GetTemperature(channel, temperature);
}

AntennaMiniportMock::AntennaMiniportMock()
{
    auto This = static_cast<AntennaMiniportDriver*>(this);
    This->Reset = ::Reset;
    This->ArmDeploymentSystem = ::ArmDeploymentSystem;
    This->DisarmDeploymentSystem = ::DisarmDeploymentSystem;
    This->DeployAntenna = ::DeployAntenna;
    This->InitializeAutomaticDeployment = ::InitializeAutomaticDeployment;
    This->CancelAntennaDeployment = ::CancelAntennaDeployment;
    This->GetDeploymentStatus = ::GetDeploymentStatus;
    This->GetAntennaActivationCount = ::GetAntennaActivationCount;
    This->GetAntennaActivationTime = ::GetAntennaActivationTime;
    This->GetTemperature = ::GetTemperature;
}

AntennaMiniportMock::~AntennaMiniportMock()
{
}
