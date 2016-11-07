#include "miniport.h"
#include <string.h>
#include <cstdint>
#include "base/reader.h"
#include "logger/logger.h"

/**
 * @brief Enumerator of all supported antenna controller commands.
 * @ingroup AntennaMiniport
 */
enum Command
{
    DEPLOY_ANTENNA = 0xa0,
    START_AUTOMATIC_DEPLOYMENT = 0xa5,
    CANCEL_ANTENNA_DEPLOYMENT = 0xa9,
    RESET = 0xaa,
    DISARM = 0xac,
    ARM = 0xad,
    DEPLOY_ANTENNA_OVERRIDE = 0xb9,
    QUERY_ANTENNA_ACTIVATION_COUNT = 0xaf,
    QUERY_ANTENNA_ACTIVATION_TIME = 0xb3,
    QUERY_TEMPERATURE = 0xc0,
    QUERY_DEPLOYMENT_STATUS = 0xc3
};

/**
 * @brief Maps boolean operation status to OSResult.
 * @param[in] status Operation status.
 * @return Mapped operation status.
 * @ingroup AntennaMiniport
 */
static inline OSResult MapStatus(bool status)
{
    return status ? OSResultSuccess : OSResultIOError;
}

/**
 * @brief Sends requested command via the passed miniport driver.
 * @param[in] driver Driver object that should be used to send the requested command.
 * @param[in] command Command to be sent.
 * @return Operation status. True on success, false otherwise.
 * @ingroup AntennaMiniport
 */
static bool SendCommand(I2CBus* bus, AntennaChannel channel, Command command)
{
    uint8_t data = (uint8_t)command;
    const I2CResult result = bus->Write(bus, channel, &data, 1);
    const bool status = (result == I2CResultOK);
    if (!status)
    {
        LOGF(LOG_LEVEL_ERROR, "[ant] Unable to send command %d to %d, Reason: %d", command, channel, result);
    }

    return status;
}

/**
 * @brief Sends requested command via the passed miniport driver and reads hardware response
 * @param[in] driver Driver object that should be used to send the requested command.
 * @param[in] command Command to be sent.
 * @param[out] outBuffer Buffer that should be filled with response from hardware.
 * @param[in] outBufferSize Size in bytes of the outBuffer buffer.
 * @return Operation status. True on success, false otherwise.
 * @ingroup AntennaMiniport
 */
static bool SendCommandWithResponse(I2CBus* bus,
    AntennaChannel channel,
    Command command,
    uint8_t* outBuffer,
    uint8_t outBufferSize //
    )
{
    const I2CResult result =
        bus->WriteRead(bus, channel, reinterpret_cast<std::uint8_t*>(&command), sizeof(command), outBuffer, outBufferSize);
    const bool status = (result == I2CResultOK);
    if (!status)
    {
        LOGF(LOG_LEVEL_ERROR, "[ant] Unable to send command %d to %d, Reason: %d", command, channel, result);
    }

    return status;
}

static OSResult Reset(AntennaMiniportDriver* miniport,
    I2CBus* communicationBus,
    AntennaChannel channel //
    )
{
    UNREFERENCED_PARAMETER(miniport);
    return MapStatus(SendCommand(communicationBus, channel, RESET));
}

static OSResult ArmDeploymentSystem(AntennaMiniportDriver* miniport,
    I2CBus* communicationBus,
    AntennaChannel channel //
    )
{
    UNREFERENCED_PARAMETER(miniport);
    return MapStatus(SendCommand(communicationBus, channel, ARM));
}

static OSResult DisarmDeploymentSystem(AntennaMiniportDriver* miniport,
    I2CBus* communicationBus,
    AntennaChannel channel //
    )
{
    UNREFERENCED_PARAMETER(miniport);
    return MapStatus(SendCommand(communicationBus, channel, DISARM));
}

static inline uint8_t GetOverrideFlag(bool override)
{
    return override ? (DEPLOY_ANTENNA_OVERRIDE - DEPLOY_ANTENNA) : 0;
}

static OSResult DeployAntenna(struct AntennaMiniportDriver* miniport,
    I2CBus* communicationBus,
    AntennaChannel channel,
    AntennaId antennaId,
    TimeSpan timeout,
    bool override //
    )
{
    UNREFERENCED_PARAMETER(miniport);
    uint8_t buffer[2];
    buffer[0] = (uint8_t)(DEPLOY_ANTENNA + antennaId + GetOverrideFlag(override));
    buffer[1] = (uint8_t)TimeSpanToSeconds(timeout);
    return MapStatus(communicationBus->Write(communicationBus,
                         channel,
                         buffer,
                         sizeof(buffer) //
                         ) == I2CResultOK);
}

static OSResult InitializeAutomaticDeployment(AntennaMiniportDriver* miniport,
    I2CBus* communicationBus,
    AntennaChannel channel //
    )
{
    UNREFERENCED_PARAMETER(miniport);
    return MapStatus(SendCommand(communicationBus, channel, START_AUTOMATIC_DEPLOYMENT));
}

static OSResult CancelAntennaDeployment(AntennaMiniportDriver* miniport,
    I2CBus* communicationBus,
    AntennaChannel channel //
    )
{
    UNREFERENCED_PARAMETER(miniport);
    return MapStatus(SendCommand(communicationBus, channel, CANCEL_ANTENNA_DEPLOYMENT));
}

static OSResult GetDeploymentStatus(AntennaMiniportDriver* miniport,
    I2CBus* communicationBus,
    AntennaChannel channel,
    AntennaDeploymentStatus* telemetry //
    )
{
    UNREFERENCED_PARAMETER(miniport);
    uint8_t output[2];
    memset(telemetry, 0, sizeof(*telemetry));
    if (!SendCommandWithResponse(communicationBus, channel, QUERY_DEPLOYMENT_STATUS, output, sizeof(output)))
    {
        return OSResultIOError;
    }

    Reader reader;
    ReaderInitialize(&reader, output, sizeof(output));
    const uint16_t value = ReaderReadWordLE(&reader);
    if ((value & 0x1000) != 0)
    {
        LOGF(LOG_LEVEL_WARNING,
            "[ant] Antenna %d deployment status out of range: %d.",
            channel,
            value //
            );

        return OSResultOutOfRange;
    }

    telemetry->DeploymentStatus[0] = (value & 0x8000) != 0;
    telemetry->DeploymentStatus[1] = (value & 0x0800) != 0;
    telemetry->DeploymentStatus[2] = (value & 0x0080) != 0;
    telemetry->DeploymentStatus[3] = (value & 0x0008) != 0;

    telemetry->IsDeploymentActive[0] = (value & 0x2000) != 0;
    telemetry->IsDeploymentActive[1] = (value & 0x0200) != 0;
    telemetry->IsDeploymentActive[2] = (value & 0x0020) != 0;
    telemetry->IsDeploymentActive[3] = (value & 0x0002) != 0;
    telemetry->IgnoringDeploymentSwitches = (value & 0x0100) != 0;
    telemetry->DeploymentSystemArmed = (value & 0x0001) != 0;
    return OSResultSuccess;
}

static OSResult GetAntennaActivationCount(AntennaMiniportDriver* miniport,
    I2CBus* communicationBus,
    AntennaChannel channel,
    AntennaId antennaId,
    uint16_t* count //
    )
{
    UNREFERENCED_PARAMETER(miniport);
    uint8_t output;
    if (!SendCommandWithResponse(communicationBus, channel, (Command)(QUERY_ANTENNA_ACTIVATION_COUNT + antennaId), &output, sizeof(output)))
    {
        *count = 0;
        return OSResultIOError;
    }

    *count = output;
    return OSResultSuccess;
}

static OSResult GetAntennaActivationTime(AntennaMiniportDriver* miniport,
    I2CBus* communicationBus,
    AntennaChannel channel,
    AntennaId antennaId,
    TimeSpan* span //
    )
{
    UNREFERENCED_PARAMETER(miniport);
    uint8_t output[2];
    if (!SendCommandWithResponse(communicationBus, channel, (Command)(QUERY_ANTENNA_ACTIVATION_TIME + antennaId), output, sizeof(output)))
    {
        *span = TimeSpanFromMilliseconds(0);
        return OSResultIOError;
    }

    Reader reader;
    ReaderInitialize(&reader, output, sizeof(output));
    const uint16_t value = ReaderReadWordBE(&reader);
    *span = TimeSpanFromMilliseconds(value * 50);
    return OSResultSuccess;
}

static OSResult GetTemperature(AntennaMiniportDriver* miniport,
    I2CBus* communicationBus,
    AntennaChannel channel,
    uint16_t* temperature //
    )
{
    UNREFERENCED_PARAMETER(miniport);
    uint8_t output[2];
    *temperature = 0;
    if (!SendCommandWithResponse(communicationBus, channel, QUERY_TEMPERATURE, output, sizeof(output)))
    {
        return OSResultIOError;
    }

    Reader reader;
    ReaderInitialize(&reader, output, sizeof(output));
    const uint16_t value = ReaderReadWordBE(&reader);
    if ((value & 0xfc00) != 0)
    {
        LOGF(LOG_LEVEL_WARNING,
            "[ant] Antenna %d temperature is out of range: %d.",
            channel,
            value //
            );

        return OSResultOutOfRange;
    }

    *temperature = value & 0x3ff;
    return OSResultSuccess;
}

void AntennaMiniportInitialize(AntennaMiniportDriver* driver)
{
    memset(driver, 0, sizeof(*driver));
    driver->Reset = Reset;
    driver->ArmDeploymentSystem = ArmDeploymentSystem;
    driver->DisarmDeploymentSystem = DisarmDeploymentSystem;
    driver->DeployAntenna = DeployAntenna;
    driver->InitializeAutomaticDeployment = InitializeAutomaticDeployment;
    driver->CancelAntennaDeployment = CancelAntennaDeployment;
    driver->GetDeploymentStatus = GetDeploymentStatus;
    driver->GetAntennaActivationCount = GetAntennaActivationCount;
    driver->GetAntennaActivationTime = GetAntennaActivationTime;
    driver->GetTemperature = GetTemperature;
}
