/**
@file Driver for communication module.

@remarks Based on ICD Issue 1.0 2014-12-19
*/
#include "comm.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include "base/os.h"
#include "base/reader.h"
#include "base/writer.h"
#include "logger/logger.h"
#include "system.h"

typedef enum {
    CommReceiver = 0x60,
    CommTransmitter = 0x62,
} CommAddress;

typedef enum {
    CommHardReset = 0xAB,
    ReceiverSoftReset = 0xAA,
    ReceiverGetFrameCount = 0x21,
    ReceiverGetFrame = 0x22,
    ReceiverRemoveFrame = 0x24,
    ReceiverGetTelemetry = 0x1A,
} ReceiverCommand;

typedef enum {
    TransmitterSoftReset = 0xAA,
    TransmitterSendFrame = 0x10,
    TransmitterGetTelemetry = 0x25,
    TransmitterSetBeacon = 0x14,
    TransmitterClearBeacon = 0x1f,
    TransmitterSetIdleState = 0x24,
    TransmitterSetBitRate = 0x28,
    TransmitterGetState = 0x41
} TransmitterCommand;

typedef enum {
    TaskFlagPauseRequest = 1,
    TaskFlagAck = 2,
} TaskFlag;

static noreturn void CommTask(void* param);

static bool SendCommand(CommObject* object, CommAddress address, uint8_t command)
{
    const I2CResult result = object->low->Write(object->low, address, &command, sizeof(command));
    const bool status = (result == I2CResultOK);
    if (!status)
    {
        LOGF(LOG_LEVEL_ERROR, "[comm] Unable to send command %d to %d, Reason: %d", command, address, result);
    }

    return status;
}

static bool SendCommandWithResponse(CommObject* object, CommAddress address, uint8_t command, uint8_t* outBuffer, uint8_t outBufferSize)
{
    const I2CResult result = object->low->WriteRead(object->low, address, &command, sizeof(command), outBuffer, outBufferSize);
    const bool status = (result == I2CResultOK);
    if (!status)
    {
        LOGF(LOG_LEVEL_ERROR, "[comm] Unable to send command %d to %d, Reason: %d", command, address, result);
    }

    return status;
}

OSResult CommInitialize(CommObject* comm, I2CBus* i2c, CommUpperInterface* upperInterface)
{
    memset(comm, 0, sizeof(CommObject));
    comm->low = i2c;
    comm->upper = *upperInterface;
    comm->commTaskFlags = System.CreateEventGroup();
    if (comm->commTaskFlags != NULL)
    {
        return OSResultSuccess;
    }
    else
    {
        return OSResultNotEnoughMemory;
    }
}

bool CommRestart(CommObject* comm)
{
    if (!CommReset(comm))
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable reset comm hardware. ");
        return false;
    }

    if (comm->commTask == NULL)
    {
        const OSResult result = System.CreateTask(CommTask, "COMM Task", 512, comm, 4, &comm->commTask);
        if (result != OSResultSuccess)
        {
            LOGF(LOG_LEVEL_ERROR, "[comm] Unable to create background task. Status: 0x%08x.", result);
            return false;
        }
    }

    return true;
}

bool CommPause(CommObject* comm)
{
    if (comm->commTask != NULL)
    {
        System.EventGroupSetBits(comm->commTaskFlags, TaskFlagPauseRequest);
        System.EventGroupWaitForBits(comm->commTaskFlags, TaskFlagAck, false, true, MAX_DELAY);
    }

    return true;
}

bool CommReset(CommObject* comm)
{
    return SendCommand(comm, CommReceiver, CommHardReset) && CommResetReceiver(comm) && CommResetTransmitter(comm);
}

bool CommResetTransmitter(CommObject* comm)
{
    return SendCommand(comm, CommTransmitter, TransmitterSoftReset);
}

bool CommResetReceiver(CommObject* comm)
{
    return SendCommand(comm, CommReceiver, ReceiverSoftReset);
}

CommReceiverFrameCount CommGetFrameCount(CommObject* comm)
{
    CommReceiverFrameCount result;
    uint8_t count = 0;
    result.status = SendCommandWithResponse(comm, CommReceiver, ReceiverGetFrameCount, &count, sizeof(count));
    if (result.status)
    {
        LOGF(LOG_LEVEL_INFO, "There are %d frames.", (int)count);
        result.frameCount = count;
    }
    else
    {
        LOG(LOG_LEVEL_ERROR, "Unable to get frame count");
        result.frameCount = 0;
    }

    return result;
}

bool CommRemoveFrame(CommObject* comm)
{
    const bool status = SendCommand(comm, CommReceiver, ReceiverRemoveFrame);
    if (!status)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Failed to remove frame from buffer");
    }

    return status;
}

bool CommGetReceiverTelemetry(CommObject* comm, CommReceiverTelemetry* telemetry)
{
    uint8_t buffer[sizeof(CommReceiverTelemetry)];
    const bool status = SendCommandWithResponse(comm, CommReceiver, ReceiverGetTelemetry, buffer, COUNT_OF(buffer));
    if (!status)
    {
        return status;
    }

    Reader reader;
    ReaderInitialize(&reader, buffer, COUNT_OF(buffer));
    telemetry->TransmitterCurrentConsumption = ReaderReadWordLE(&reader);
    telemetry->DopplerOffset = ReaderReadWordLE(&reader);
    telemetry->ReceiverCurrentConsumption = ReaderReadWordLE(&reader);
    telemetry->Vcc = ReaderReadWordLE(&reader);
    telemetry->OscilatorTemperature = ReaderReadWordLE(&reader);
    telemetry->AmplifierTemperature = ReaderReadWordLE(&reader);
    telemetry->SignalStrength = ReaderReadWordLE(&reader);

    if ((telemetry->TransmitterCurrentConsumption & 0xf000) != 0 || (telemetry->DopplerOffset & 0xf000) != 0 ||
        (telemetry->ReceiverCurrentConsumption & 0xf000) != 0 || (telemetry->Vcc & 0xf000) != 0 ||
        (telemetry->OscilatorTemperature & 0xf000) != 0 || (telemetry->AmplifierTemperature & 0xf000) != 0 ||
        (telemetry->SignalStrength & 0xf000) != 0)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Received invalid receiver telemetry. ");
        return false;
    }

    return ReaderStatus(&reader);
}

bool CommGetTransmitterTelemetry(CommObject* comm, CommTransmitterTelemetry* telemetry)
{
    uint8_t buffer[sizeof(CommTransmitterTelemetry)];
    const bool status = SendCommandWithResponse(comm, CommTransmitter, TransmitterGetTelemetry, buffer, COUNT_OF(buffer));
    if (!status)
    {
        return status;
    }

    Reader reader;
    ReaderInitialize(&reader, buffer, COUNT_OF(buffer));
    telemetry->RFReflectedPower = ReaderReadWordLE(&reader);
    telemetry->AmplifierTemperature = ReaderReadWordLE(&reader);
    telemetry->RFForwardPower = ReaderReadWordLE(&reader);
    telemetry->TransmitterCurrentConsumption = ReaderReadWordLE(&reader);

    if ((telemetry->RFReflectedPower & 0xf000) != 0 || (telemetry->AmplifierTemperature & 0xf000) != 0 ||
        (telemetry->RFForwardPower & 0xf000) != 0 || (telemetry->TransmitterCurrentConsumption & 0xf000) != 0)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Received invalid transmitter telemetry. ");
        return false;
    }

    return ReaderStatus(&reader);
}

bool CommReceiveFrame(CommObject* comm, CommFrame* frame)
{
    uint8_t buffer[COMM_MAX_FRAME_CONTENTS_SIZE + 20];
    bool status = SendCommandWithResponse(comm, CommReceiver, ReceiverGetFrame, buffer, COUNT_OF(buffer));
    if (!status)
    {
        return status;
    }

    Reader reader;
    ReaderInitialize(&reader, buffer, COUNT_OF(buffer));
    frame->Size = ReaderReadWordLE(&reader);
    frame->Doppler = ReaderReadWordLE(&reader);
    frame->RSSI = ReaderReadWordLE(&reader);
    const uint8_t* data = ReaderReadArray(&reader, frame->Size);

    if (frame->Size > COMM_MAX_FRAME_CONTENTS_SIZE)
    {
        LOGF(LOG_LEVEL_ERROR, "[comm] Invalid frame length: %d ", (int)frame->Size);
        return false;
    }

    if (data != NULL)
    {
        memcpy(frame->Contents, data, frame->Size);
    }

    status = ReaderStatus(&reader);
    if (!status)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Failed to receive frame");
    }
    else
    {
        LOGF(LOG_LEVEL_DEBUG, "[comm] Received frame %d bytes", (int)frame->Size);
    }

    if (frame->Size == 0 || (frame->Doppler & 0xf000) != 0 || (frame->RSSI & 0xf000) != 0)
    {
        LOGF(LOG_LEVEL_ERROR,
            "[comm] Received invalid frame. Size: %d, Doppler: 0x%X, RSSI: 0x%X. ",
            frame->Size,
            frame->Doppler,
            frame->RSSI);
        return false;
    }

    return status;
}

bool CommSendFrame(CommObject* comm, uint8_t* data, uint8_t length)
{
    uint8_t cmd[255];
    if (length > COMM_MAX_FRAME_CONTENTS_SIZE)
    {
        LOGF(LOG_LEVEL_ERROR, "Frame payload is too long. Allowed: %d, Requested: '%d'.", COMM_MAX_FRAME_CONTENTS_SIZE, length);
        return false;
    }

    cmd[0] = TransmitterSendFrame;
    memcpy(cmd + 1, data, length);
    uint8_t remainingBufferSize;

    const bool status = (comm->low->WriteRead(comm->low, CommTransmitter, cmd, length + 1, &remainingBufferSize, 1) == I2CResultOK);
    if (!status)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Failed to send frame");
    }

    if (remainingBufferSize == 0xff)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Frame was not accepted by the transmitter.");
    }

    return status && remainingBufferSize != 0xff;
}

bool CommSetBeacon(CommObject* comm, const CommBeacon* beaconData)
{
    uint8_t buffer[COMM_MAX_FRAME_CONTENTS_SIZE + 2];
    Writer writer;
    WriterInitialize(&writer, buffer, COUNT_OF(buffer));
    WriterWriteByte(&writer, TransmitterSetBeacon);
    WriterWriteWordLE(&writer, beaconData->Period);
    WriterWriteArray(&writer, beaconData->Data, beaconData->DataSize);
    if (!WriterStatus(&writer))
    {
        return false;
    }

    return comm->low->Write(comm->low, CommTransmitter, buffer, WriterGetDataLength(&writer)) == I2CResultOK;
}

bool CommClearBeacon(CommObject* comm)
{
    return SendCommand(comm, CommTransmitter, TransmitterClearBeacon);
}

bool CommSetTransmitterStateWhenIdle(CommObject* comm, CommTransmitterIdleState requestedState)
{
    uint8_t buffer[2];
    buffer[0] = TransmitterSetIdleState;
    buffer[1] = requestedState;
    return comm->low->Write(comm->low, CommTransmitter, buffer, COUNT_OF(buffer)) == I2CResultOK;
}

bool CommSetTransmitterBitRate(CommObject* comm, CommTransmitterBitrate bitrate)
{
    uint8_t buffer[2];
    buffer[0] = TransmitterSetBitRate;
    buffer[1] = bitrate;
    return comm->low->Write(comm->low, CommTransmitter, buffer, COUNT_OF(buffer)) == I2CResultOK;
}

bool CommGetTransmitterState(CommObject* comm, CommTransmitterState* state)
{
    uint8_t command = TransmitterGetState;
    uint8_t response;
    const bool status =
        (comm->low->WriteRead(comm->low, CommTransmitter, &command, sizeof(command), &response, sizeof(response)) == I2CResultOK);
    if (!status)
    {
        return false;
    }

    if ((response & 0x80) != 0)
    {
        LOG(LOG_LEVEL_ERROR, "Received invalid transmitter status. ");
        return false;
    }

    state->BeaconState = (response & 2) != 0;
    state->StateWhenIdle = (CommTransmitterIdleState)(response & 1);
    static const CommTransmitterBitrate conversionArray[] = {
        Comm1200bps, Comm2400bps, Comm4800bps, Comm9600bps,
    };

    state->TransmitterBitRate = conversionArray[(response & 0x0c) >> 2];
    return true;
}

static void CommPollHardware(CommObject* comm)
{
    CommReceiverFrameCount frameResponse = CommGetFrameCount(comm);
    if (!frameResponse.status)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to get receiver frame count. ");
    }
    else if (frameResponse.frameCount > 0)
    {
        LOGF(LOG_LEVEL_INFO, "[comm] Got %d frames", frameResponse.frameCount);

        for (uint8_t i = 0; i < frameResponse.frameCount; i++)
        {
            CommFrame frame;
            bool status = CommReceiveFrame(comm, &frame);
            if (!status)
            {
                LOG(LOG_LEVEL_ERROR, "[comm] Unable to receive frame. ");
            }
            else
            {
                if (!CommRemoveFrame(comm))
                {
                    LOG(LOG_LEVEL_ERROR, "[comm] Unable to remove frame from receiver. ");
                }

                LOGF(LOG_LEVEL_INFO, "[comm] Received frame %d bytes. ", (int)frame.Size);
                comm->upper.frameHandler(comm, &frame, comm->upper.frameHandlerContext);
            }
        }
    }
}

static void CommTask(void* param)
{
    CommObject* comm = (CommObject*)param;
    CommPollHardware(comm);
    for (;;)
    {
        const OSEventBits result = System.EventGroupWaitForBits(comm->commTaskFlags, TaskFlagPauseRequest, false, true, 10000);
        if (result == TaskFlagPauseRequest)
        {
            LOG(LOG_LEVEL_WARNING, "Comm task paused");
            System.EventGroupSetBits(comm->commTaskFlags, TaskFlagAck);
            System.SuspendTask(NULL);
        }
        else
        {
            CommPollHardware(comm);
        }
    }
}
