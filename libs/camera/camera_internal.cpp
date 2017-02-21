#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <stdio.h>
#include <string.h>

#include "camera.h"
#include "leuart/leuart.h"
#include "logger/logger.h"
#include "system.h"
#include "uart/uart.h"

using namespace devices::camera;
static uint8_t cmd[6];
static uint8_t retcmd[12];

void Camera::CameraLogSendCmd(uint8_t* cmd)
{
    LOGF(LOG_LEVEL_INFO, "Command send:%02X%02X%02X%02X%02X%02X", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5]);
}

void Camera::CameraLogGetCmd(uint8_t* cmd)
{
    LOGF(LOG_LEVEL_INFO, "Command received:%02X%02X%02X%02X%02X%02X", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5]);
}

void Camera::CameraSendCmd(uint8_t* cmd, uint8_t length)
{
	_uartBus.Write(gsl::span<uint8_t>(cmd,length));
}

uint32_t Camera::CameraGetData(uint8_t* data, uint32_t dataLength, int8_t timeoutMs,bool send)
{
    //uint8_t byte;
    TickType_t ticks = timeoutMs < 0 ? portMAX_DELAY : timeoutMs / portTICK_PERIOD_MS;
    (void)ticks;
    if(send == true){
    _uartBus.Read(gsl::span<uint8_t>(data,dataLength));
    }
    return dataLength;
}

bool Camera::CameraGetCmd( uint8_t* cmd, uint32_t length, int8_t timeoutMs,bool send)
{
    if (CameraGetData(cmd, length, timeoutMs,send) < length)
    {
        LOG(LOG_LEVEL_ERROR, "Too less data received.");
        return false;
    }

    CameraLogGetCmd(cmd);
    return true;
}

bool Camera::CameraGetCmdSync()
{

    if (!CameraGetCmd(retcmd, CameraRetCmdLength, -1,true))
    {
        LOG(LOG_LEVEL_ERROR, "Receiving data failed.");
        return false;
    }

    if (CameraParseSyncCmd(retcmd, CameraRetCmdLength) != CameraCmd::Sync)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid command received. Sync cmd was expected");
        return false;
    }

    return true;
}

bool Camera::CameraGetCmdData(CameraCmdData* cmdData)
{

    if (!CameraGetCmd(retcmd, CameraRetCmdLength, -1,false))
    {
        LOG(LOG_LEVEL_ERROR, "Receiving data failed.");
        return false;
    }

    if (CameraParseDataCmd(retcmd, CameraRetCmdLength, cmdData) != CameraCmd::Data)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid command received. Data cmd was expected. ");
        return false;
    }

    return true;
}

CameraCmd Camera::CameraGetCmdAck(int8_t timeoutMs, uint8_t length)
{
    CameraCmdAck ackData;
    if (!CameraCmdAckInit(&ackData))
    {
        LOG(LOG_LEVEL_ERROR, "Initialiazing ack data failed.");
        return CameraCmd::Invalid;
    }

    if (!CameraGetCmd(retcmd, length, timeoutMs,true))
    {
        LOG(LOG_LEVEL_ERROR, "Receiving data failed.");
        return CameraCmd::Invalid;
    }

    if (CameraParseAckCmd(retcmd, length, &ackData) != CameraCmd::Ack)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid command received. Ack cmd was expected.");
        return CameraCmd::Invalid;
    }

    return ackData.type;
}

bool Camera::CameraGetCmdAckSync(int8_t timeoutMs)
{
    if (CameraGetCmdAck(timeoutMs,CameraRetCmdLength) != CameraCmd::Sync)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid ack command received. Sync ack was expected.");
        return false;
    }
    return true;
}

bool Camera::CameraGetCmdAckInitial()
{
    if (CameraGetCmdAck(-1,CameraCmdLength) != CameraCmd::Initial)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid ack command received. Initial ack was expected.");
        return false;
    }
    return true;
}

bool Camera::CameraGetCmdAckSnapshot()
{
    if (CameraGetCmdAck(-1,CameraRetCmdLength) != CameraCmd::Snapshot)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid ack command received. Snapshot ack was expected.");
        return false;
    }
    return true;
}

bool Camera::CameraGetCmdAckGetPicture()
{
    if (CameraGetCmdAck(-1,CameraRetCmdLength) != CameraCmd::GetPicture)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid ack command received. GetPicture ack was expected.");
        return false;
    }
    return true;
}

bool Camera::CameraGetCmdAckSetPackageSize()
{
    if (CameraGetCmdAck(-1,CameraCmdLength) != CameraCmd::SetPackageSize)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid ack command received. SetPackageSize was expected.");
        return false;
    }
    return true;
}

void Camera::CameraSendCmdAck(CameraCmd cmdAck, uint8_t packageIdLow, uint8_t packageIdHigh)
{
    cmd[0] = CameraCmdPrefix;
    cmd[1]= (uint8_t)CameraCmd::Ack;
    cmd[2]=(uint8_t)cmdAck;
    cmd[3]=0x00;
    cmd[4]=packageIdLow;
    cmd[5]=packageIdHigh;
    CameraLogSendCmd(cmd);
    CameraSendCmd(cmd, CameraCmdLength);
}

void Camera::CameraSendCmdAckSync(void)
{
    LOG(LOG_LEVEL_INFO, "Sending Sync Ack command.");
    CameraSendCmdAck(CameraCmd::Sync, 0x00, 0x00);
}

void Camera::CameraSendCmdAckData(void)
{
    LOG(LOG_LEVEL_INFO, "Sending Data Ack command.");
    CameraSendCmdAck(CameraCmd::Data, 0x00, 0x00);
}

void Camera::CameraSendCmdSync(void)
{
	 	cmd[0] = CameraCmdPrefix;
	    cmd[1]= (uint8_t)CameraCmd::Sync;
	    cmd[2]=0x00;
	    cmd[3]=0x00;
	    cmd[4]=0x00;
	    cmd[5]=0x00;

    LOG(LOG_LEVEL_INFO, "Sending Sync command.");
    CameraLogSendCmd(cmd);
    CameraSendCmd(cmd, CameraCmdLength);
}
void Camera::CameraSendCmdReset(void)
{
		cmd[0] = CameraCmdPrefix;
	    cmd[1]= (uint8_t)CameraCmd::Reset;
	    cmd[2]=0x00;
	    cmd[3]=0x00;
	    cmd[4]=0x00;
	    cmd[5]=0xFF;

    LOG(LOG_LEVEL_INFO, "Sending Rest command.");
    CameraLogSendCmd(cmd);
    CameraSendCmd(cmd, CameraCmdLength);
}

void Camera::CameraSendCmdRAWInitial(CameraRAWImageFormat format, CameraRAWResolution rawResolution)
{
	 cmd[0] = CameraCmdPrefix;
		    cmd[1]= (uint8_t)CameraCmd::Initial;
		    cmd[2]=0x00;
		    cmd[3]=(uint8_t)format;
		    cmd[4]=(uint8_t)rawResolution;
		    cmd[5]=0x00;

    LOG(LOG_LEVEL_INFO, "Sending Inital command.");
    CameraLogSendCmd(cmd);
    CameraSendCmd(cmd, CameraCmdLength);
}

void Camera::CameraSendCmdJPEGInitial(CameraJPEGResolution jpegResolution)
{
	cmd[0] = CameraCmdPrefix;
			    cmd[1]= (uint8_t)CameraCmd::Initial;
			    cmd[2]=0x00;
			    cmd[3]=CameraJPEGFormat;
			    cmd[4]=0x07;
			    cmd[5]=(uint8_t)jpegResolution;

    LOG(LOG_LEVEL_INFO, "Sending Inital command.");
    CameraLogSendCmd(cmd);
    CameraSendCmd(cmd, CameraCmdLength);
}

void Camera::CameraSendCmdGetPicture(CameraPictureType type)
{
	cmd[0] = CameraCmdPrefix;
				    cmd[1]= (uint8_t)CameraCmd::GetPicture;
				    cmd[2]=(uint8_t)type;
				    cmd[3]=0x00;
				    cmd[4]=0x00;
				    cmd[5]=0x00;

    LOG(LOG_LEVEL_INFO, "Sending GetPicture command.");
    CameraLogSendCmd(cmd);
    CameraSendCmd(cmd, CameraCmdLength);
}

void Camera::CameraSendCmdSnapshot(CameraSnapshotType type)
{
	cmd[0] = CameraCmdPrefix;
					    cmd[1]= (uint8_t)CameraCmd::Snapshot;
					    cmd[2]=(uint8_t)type;
					    cmd[3]=0x00;
					    cmd[4]=0x00;
					    cmd[5]=0x00;


    LOG(LOG_LEVEL_INFO, "Sending Snapshot command.");
    CameraLogSendCmd(cmd);
    CameraSendCmd(cmd, CameraCmdLength);
}

bool Camera::CameraSendCmdSetPackageSize(uint16_t packageSize)
{
    if (packageSize > 512 || packageSize < 64)
    {
        LOG(LOG_LEVEL_ERROR, "Package size is invalid. It should be smaller than 512 and larger than 64.");
        return false;
    }


    cmd[0] = CameraCmdPrefix;
    					    cmd[1]= (uint8_t)CameraCmd::SetPackageSize;
    					    cmd[2]=0x08;
    					    cmd[3]=(uint8_t)(packageSize & 0xFF);
    					    cmd[4]=(uint8_t)((packageSize >> 8) & 0xFF);
    					    cmd[5]=0x00;


    LOG(LOG_LEVEL_INFO, "Sending SetPackageSize command.");
    CameraLogSendCmd(cmd);
    CameraSendCmd(cmd, CameraCmdLength);
    return true;
}

// Sync command is send with delay of 5 ms.
// Delay is incremented after every command send.
// Sync command may be send from 25-60 times.
bool Camera::CameraSync()
{
    int8_t i = 60;
    int8_t timeMs = 5;

    while (i > 0)
    {
        CameraSendCmdSync();
        i--;

        if (!CameraGetCmdAckSync( timeMs))
        {
            timeMs++;
            continue;
        }
        LOG(LOG_LEVEL_INFO, "Received Sync Ack Cmd.");
        if (!CameraGetCmdSync())
                      {
                          LOG(LOG_LEVEL_ERROR, "Receiving Sync command failed.");
                          return false;
                      }
                      LOG(LOG_LEVEL_INFO, "Received Sync Cmd.");
        // If we received data and it's ack for our sync command we wait for sync cmd


        // We get sync command, so we send ack for this sync and synchronization is done
        Camera::CameraSendCmdAckSync();
        return true;
    }
    return false;
}

uint32_t Camera::CameraReceiveData(uint8_t* data, uint32_t dataLength)
{
    uint32_t ret = 0;
    uint32_t toLoad = 0;
    CameraCmdData cmdData;

    if (!CameraCmdDataInit(&cmdData))
    {
        LOG(LOG_LEVEL_ERROR, "Initializing Data structure failed.");
        return 0;
    }

    if (!CameraGetCmdData(&cmdData))
    {
        LOG(LOG_LEVEL_ERROR, "Receiving Data command failed.");
        return 0;
    }

    if (cmdData.type != CameraPictureType::RAW)
    {
        LOG(LOG_LEVEL_ERROR, "Received invalid PictureType.");
        return 0;
    }

    if (cmdData.dataLength <= 0)
    {
        LOG(LOG_LEVEL_ERROR, "Received invalid picture size.");
        return 0;
    }

    if (cmdData.dataLength > dataLength)
    {
        //LOGF(LOG_LEVEL_ERROR, "Data buffer is to small. Passed:%d. Expected size:%d", dataLength, cmdData.dataLength);
        return 0;
    }

    LOG(LOG_LEVEL_INFO, "Start receiving data.");
    toLoad = cmdData.dataLength;
    while (toLoad > 0)
    {
        ret = CameraGetData(&data[cmdData.dataLength - toLoad], toLoad, -1,true);
        if (ret > 0)
        {
            toLoad -= ret;
        }
        else
        {
            LOG(LOG_LEVEL_INFO, "Getting data failed. We keep trying.");
        }
    }
    LOG(LOG_LEVEL_INFO, "Finish receiving data.");
    return cmdData.dataLength;
}

int8_t Camera::CameraReceiveJPEGData(uint8_t* data, uint16_t dataLength, uint16_t packageSize)
{
    uint16_t ret = 0;
    uint8_t i = 0;

    if (packageSize > 512 || packageSize < 64)
    {
        LOG(LOG_LEVEL_ERROR, "Package size is invalid. It should be smaller than 512 and larger than 64.");
        return false;
    }

    uint8_t packageCnt = dataLength / (packageSize - 6) + (dataLength % (packageSize - 6) != 0 ? 1 : 0);
    for (i = 0; i < packageCnt; i++)
    {
            ret = CameraGetData(&data[i*packageSize], packageSize, -1,true);
            if (ret > 0)
            {
                CameraSendCmdAck(CameraCmd::None,i & 0xff,(uint8_t)(i >> 8));
            }
    }
    return i;
}

