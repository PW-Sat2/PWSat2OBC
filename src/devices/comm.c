#include <FreeRTOS.h>
#include <task.h>

#include "comm.h"

#include <stdbool.h>
#include "Logger/Logger.h"

#include "i2c/i2c.h"
#include "system.h"

#define TRANSMITTER_ADDRESS 0x62
#define RECEIVER_ADDRESS 0x60

static bool autoCommHandling = true;

static void commTask(void* param)
{
    UNREFERENCED_PARAMETER(param);

    uint8_t cmd = 0xAA;

    if (I2CWrite(TRANSMITTER_ADDRESS, &cmd, 1) != i2cTransferDone)
    {
        LOG(LOG_LEVEL_ERROR, "Transmitter reset failed");
    }

    if (I2CWrite(RECEIVER_ADDRESS, &cmd, 1) != i2cTransferDone)
    {
        LOG(LOG_LEVEL_ERROR, "Transmitter reset failed");
    }

    LOG(LOG_LEVEL_INFO, "Comm initialized");

    while (1)
    {
        if (autoCommHandling)
        {
            uint8_t framesCount = CommGetFramesCount();
            if (framesCount > 0)
            {
                LOGF(LOG_LEVEL_INFO, "Got %d frames", framesCount);

                for (uint8_t i = 0; i < framesCount; i++)
                {
                    Frame frame = {.Contents = {0}};

                    CommReceiveFrame(&frame);

                    LOGF(LOG_LEVEL_INFO, "Received frame %d bytes: %s", frame.Size, frame.Contents);

                    char msg[] = "PONG";

                    CommSendFrame(msg, 4);

                    CommRemoveFrame();
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void CommInit(void)
{
    xTaskCreate(commTask, "COMM Task", 512, NULL, 4, NULL);
}

void CommAutoHandling(bool enable)
{
	autoCommHandling = enable;
}

void CommSendFrame(uint8_t* data, uint8_t length)
{
    uint8_t cmd[255] = {0x10};

    for (uint8_t i = 0; i < length; i++)
    {
        cmd[i + 1] = data[i];
    }

    uint8_t remainingBufferSize;

    if (I2CWriteRead(TRANSMITTER_ADDRESS, cmd, length + 1, &remainingBufferSize, 1) !=
        i2cTransferDone) // TODO: handle response
    {
        LOG(LOG_LEVEL_ERROR, "Failed to send frame");
    }
}

uint8_t CommGetFramesCount(void)
{
    uint8_t cmd = 0x21;
    uint8_t count = 0;

    if (I2CWriteRead(RECEIVER_ADDRESS, &cmd, 1, &count, 1) != i2cTransferDone)
    {
        LOG(LOG_LEVEL_ERROR, "Failed to get frame count");
    }

    return count;
}

void CommReceiveFrame(Frame* frame)
{
    uint8_t cmd = 0x22;

    LOG(LOG_LEVEL_TRACE, "Receiving frame");

    if (I2CWriteRead(RECEIVER_ADDRESS, &cmd, 1, (uint8_t*)frame, sizeof(Frame)) != i2cTransferDone)
    {
        LOG(LOG_LEVEL_ERROR, "Failed to receive frame");
    }
    else
    {
        LOGF(LOG_LEVEL_DEBUG, "Received frame %d bytes", frame->Size);
    }
}

void CommRemoveFrame(void)
{
    uint8_t cmd = 0x24;

    if (I2CWrite(RECEIVER_ADDRESS, &cmd, 1) != i2cTransferDone)
    {
        LOG(LOG_LEVEL_ERROR, "Failed to remove frame from buffer");
    }
}
