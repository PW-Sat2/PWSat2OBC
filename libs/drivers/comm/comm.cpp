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

CommObject::CommObject(I2CBus& low, IHandleFrame& upperInterface)
    : low(low), //
      upper(upperInterface)
{
}

enum ReceiverCommand
{
    CommHardReset = 0xAB,
    ReceiverSoftReset = 0xAA,
    ReceiverGetFrameCount = 0x21,
    ReceiverGetFrame = 0x22,
    ReceiverRemoveFrame = 0x24,
    ReceiverGetTelemetry = 0x1A,
};

enum TransmitterCommand
{
    TransmitterSoftReset = 0xAA,
    TransmitterSendFrame = 0x10,
    TransmitterGetTelemetry = 0x25,
    TransmitterSetBeacon = 0x14,
    TransmitterClearBeacon = 0x1f,
    TransmitterSetIdleState = 0x24,
    TransmitterSetBitRate = 0x28,
    TransmitterGetState = 0x41
};

enum TaskFlag
{
    TaskFlagPauseRequest = 1,
    TaskFlagAck = 2,
};

bool CommObject::SendCommand(CommAddress address, uint8_t command)
{
    const I2CResult result = this->low.Write(&this->low, address, &command, sizeof(command));
    const bool status = (result == I2CResultOK);
    if (!status)
    {
        LOGF(LOG_LEVEL_ERROR, "[comm] Unable to send command %d to %d, Reason: %d", command, address, result);
    }

    return status;
}

bool CommObject::SendCommandWithResponse(CommAddress address, uint8_t command, uint8_t* outBuffer, uint8_t outBufferSize)
{
    const I2CResult result = this->low.WriteRead(&this->low, address, &command, sizeof(command), outBuffer, outBufferSize);
    const bool status = (result == I2CResultOK);
    if (!status)
    {
        LOGF(LOG_LEVEL_ERROR, "[comm] Unable to send command %d to %d, Reason: %d", command, address, result);
    }

    return status;
}

OSResult CommObject::Initialize()
{
    this->commTaskFlags = System.CreateEventGroup();
    if (this->commTaskFlags != NULL)
    {
        return OSResultSuccess;
    }
    else
    {
        return OSResultNotEnoughMemory;
    }
}

bool CommObject::Restart()
{
    if (!this->Reset())
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable reset comm hardware. ");
        return false;
    }

    if (this->commTask == NULL)
    {
        const OSResult result = System.CreateTask(CommObject::CommTask, "COMM Task", 512, this, 4, &this->commTask);
        if (result != OSResultSuccess)
        {
            LOGF(LOG_LEVEL_ERROR, "[comm] Unable to create background task. Status: 0x%08x.", result);
            return false;
        }
    }

    return true;
}

bool CommObject::Pause()
{
    if (this->commTask != NULL)
    {
        System.EventGroupSetBits(this->commTaskFlags, TaskFlagPauseRequest);
        System.EventGroupWaitForBits(this->commTaskFlags, TaskFlagAck, false, true, MAX_DELAY);
    }

    return true;
}

bool CommObject::Reset()
{
    return this->SendCommand(CommReceiver, CommHardReset) && this->ResetReceiver() && this->ResetTransmitter();
}

bool CommObject::ResetTransmitter()
{
    return this->SendCommand(CommTransmitter, TransmitterSoftReset);
}

bool CommObject::ResetReceiver()
{
    return this->SendCommand(CommReceiver, ReceiverSoftReset);
}

CommReceiverFrameCount CommObject::GetFrameCount()
{
    CommReceiverFrameCount result;
    uint8_t count = 0;
    result.status = this->SendCommandWithResponse(CommReceiver, ReceiverGetFrameCount, &count, sizeof(count));
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

bool CommObject::RemoveFrame()
{
    const bool status = this->SendCommand(CommReceiver, ReceiverRemoveFrame);
    if (!status)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Failed to remove frame from buffer");
    }

    return status;
}

bool CommObject::GetReceiverTelemetry(CommReceiverTelemetry& telemetry)
{
    uint8_t buffer[sizeof(CommReceiverTelemetry)];
    const bool status = this->SendCommandWithResponse(CommReceiver, ReceiverGetTelemetry, buffer, COUNT_OF(buffer));
    if (!status)
    {
        return status;
    }

    Reader reader;
    ReaderInitialize(&reader, buffer, COUNT_OF(buffer));
    telemetry.TransmitterCurrentConsumption = ReaderReadWordLE(&reader);
    telemetry.DopplerOffset = ReaderReadWordLE(&reader);
    telemetry.ReceiverCurrentConsumption = ReaderReadWordLE(&reader);
    telemetry.Vcc = ReaderReadWordLE(&reader);
    telemetry.OscilatorTemperature = ReaderReadWordLE(&reader);
    telemetry.AmplifierTemperature = ReaderReadWordLE(&reader);
    telemetry.SignalStrength = ReaderReadWordLE(&reader);

    if ((telemetry.TransmitterCurrentConsumption & 0xf000) != 0 || //
        (telemetry.DopplerOffset & 0xf000) != 0 ||                 //
        (telemetry.ReceiverCurrentConsumption & 0xf000) != 0 ||    //
        (telemetry.Vcc & 0xf000) != 0 ||                           //
        (telemetry.OscilatorTemperature & 0xf000) != 0 ||          //
        (telemetry.AmplifierTemperature & 0xf000) != 0 ||          //
        (telemetry.SignalStrength & 0xf000) != 0)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Received invalid receiver telemetry. ");
        return false;
    }

    return ReaderStatus(&reader);
}

bool CommObject::GetTransmitterTelemetry(CommTransmitterTelemetry& telemetry)
{
    uint8_t buffer[sizeof(CommTransmitterTelemetry)];
    const bool status = this->SendCommandWithResponse(CommTransmitter, TransmitterGetTelemetry, buffer, COUNT_OF(buffer));
    if (!status)
    {
        return status;
    }

    Reader reader;
    ReaderInitialize(&reader, buffer, COUNT_OF(buffer));
    telemetry.RFReflectedPower = ReaderReadWordLE(&reader);
    telemetry.AmplifierTemperature = ReaderReadWordLE(&reader);
    telemetry.RFForwardPower = ReaderReadWordLE(&reader);
    telemetry.TransmitterCurrentConsumption = ReaderReadWordLE(&reader);

    if ((telemetry.RFReflectedPower & 0xf000) != 0 ||     //
        (telemetry.AmplifierTemperature & 0xf000) != 0 || //
        (telemetry.RFForwardPower & 0xf000) != 0 ||       //
        (telemetry.TransmitterCurrentConsumption & 0xf000) != 0)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Received invalid transmitter telemetry. ");
        return false;
    }

    return ReaderStatus(&reader);
}

bool CommObject::ReceiveFrame(CommFrame& frame)
{
    uint8_t buffer[COMM_MAX_FRAME_CONTENTS_SIZE + 20];
    bool status = this->SendCommandWithResponse(CommReceiver, ReceiverGetFrame, buffer, COUNT_OF(buffer));
    if (!status)
    {
        return status;
    }

    Reader reader;
    ReaderInitialize(&reader, buffer, COUNT_OF(buffer));
    frame.Size = ReaderReadWordLE(&reader);
    frame.Doppler = ReaderReadWordLE(&reader);
    frame.RSSI = ReaderReadWordLE(&reader);
    const uint8_t* data = ReaderReadArray(&reader, frame.Size);

    if (frame.Size > COMM_MAX_FRAME_CONTENTS_SIZE)
    {
        LOGF(LOG_LEVEL_ERROR, "[comm] Invalid frame length: %d", (int)frame.Size);
        return false;
    }

    if (data != NULL)
    {
        memcpy(frame.Contents, data, frame.Size);
    }

    status = ReaderStatus(&reader);
    if (!status)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Failed to receive frame");
    }
    else
    {
        LOGF(LOG_LEVEL_DEBUG, "[comm] Received frame %d bytes", (int)frame.Size);
    }

    if (frame.Size == 0 || (frame.Doppler & 0xf000) != 0 || (frame.RSSI & 0xf000) != 0)
    {
        LOGF(
            LOG_LEVEL_ERROR, "[comm] Received invalid frame. Size: %d, Doppler: 0x%X, RSSI: 0x%X. ", frame.Size, frame.Doppler, frame.RSSI);
        return false;
    }

    return status;
}

bool CommObject::SendFrame(uint8_t* data, uint8_t length)
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

    const bool status = (this->low.WriteRead(&this->low, CommTransmitter, cmd, length + 1, &remainingBufferSize, 1) == I2CResultOK);
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

bool CommObject::SetBeacon(const CommBeacon& beaconData)
{
    uint8_t buffer[COMM_MAX_FRAME_CONTENTS_SIZE + 2];
    Writer writer;
    WriterInitialize(&writer, buffer, COUNT_OF(buffer));
    WriterWriteByte(&writer, TransmitterSetBeacon);
    WriterWriteWordLE(&writer, beaconData.Period);
    WriterWriteArray(&writer, beaconData.Data, beaconData.DataSize);
    if (!WriterStatus(&writer))
    {
        return false;
    }

    return this->low.Write(&this->low, CommTransmitter, buffer, WriterGetDataLength(&writer)) == I2CResultOK;
}

bool CommObject::ClearBeacon()
{
    return this->SendCommand(CommTransmitter, TransmitterClearBeacon);
}

bool CommObject::SetTransmitterStateWhenIdle(CommTransmitterIdleState requestedState)
{
    uint8_t buffer[2];
    buffer[0] = TransmitterSetIdleState;
    buffer[1] = requestedState;
    return this->low.Write(&this->low, CommTransmitter, buffer, COUNT_OF(buffer)) == I2CResultOK;
}

bool CommObject::SetTransmitterBitRate(CommTransmitterBitrate bitrate)
{
    uint8_t buffer[2];
    buffer[0] = TransmitterSetBitRate;
    buffer[1] = bitrate;
    return this->low.Write(&this->low, CommTransmitter, buffer, COUNT_OF(buffer)) == I2CResultOK;
}

bool CommObject::GetTransmitterState(CommTransmitterState& state)
{
    uint8_t command = TransmitterGetState;
    uint8_t response;
    const bool status =
        (this->low.WriteRead(&this->low, CommTransmitter, &command, sizeof(command), &response, sizeof(response)) == I2CResultOK);
    if (!status)
    {
        return false;
    }

    if ((response & 0x80) != 0)
    {
        LOG(LOG_LEVEL_ERROR, "Received invalid transmitter status. ");
        return false;
    }

    state.BeaconState = (response & 2) != 0;
    state.StateWhenIdle = (CommTransmitterIdleState)(response & 1);
    static const CommTransmitterBitrate conversionArray[] = {
        Comm1200bps, Comm2400bps, Comm4800bps, Comm9600bps,
    };

    state.TransmitterBitRate = conversionArray[(response & 0x0c) >> 2];
    return true;
}

void CommObject::PollHardware()
{
    CommReceiverFrameCount frameResponse = this->GetFrameCount();
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
            bool status = this->ReceiveFrame(frame);
            if (!status)
            {
                LOG(LOG_LEVEL_ERROR, "[comm] Unable to receive frame. ");
            }
            else
            {
                if (!this->RemoveFrame())
                {
                    LOG(LOG_LEVEL_ERROR, "[comm] Unable to remove frame from receiver. ");
                }

                LOGF(LOG_LEVEL_INFO, "[comm] Received frame %d bytes. ", (int)frame.Size);
                this->upper.HandleFrame(*this, frame);
            }
        }
    }
}

void CommObject::CommTask(void* param)
{
    CommObject* comm = (CommObject*)param;
    comm->PollHardware();
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
            comm->PollHardware();
        }
    }
}
