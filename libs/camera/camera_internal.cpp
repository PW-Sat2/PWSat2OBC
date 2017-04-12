#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <stdio.h>
#include <string.h>

#include "camera.h"
#include "leuart/leuart.h"
#include "logger/logger.h"
#include "system.h"

using namespace devices::camera;
using namespace drivers::uart;

uint8_t cmd[6];
uint8_t retcmd[12];


void Camera::CameraLogSendCmd(gsl::span<const uint8_t> cmd)
{
    LOGF(LOG_LEVEL_INFO, "Command send:%02X%02X%02X%02X%02X%02X", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5]);
}

void Camera::CameraLogGetCmd(gsl::span<const uint8_t> cmd)
{
    LOGF(LOG_LEVEL_INFO, "Command received:%02X%02X%02X%02X%02X%02X", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5]);
}

void Camera::CameraSendCmd(gsl::span<const uint8_t> cmd)
{
	_uartBus.Write(cmd);
}

uint32_t Camera::CameraGetData(gsl::span<const uint8_t> data ,bool send)
{

    if(send == true){
   _uartBus.Read(data);
    }
    return data.length();
}

bool Camera::CameraGetCmd(gsl::span<const uint8_t> cmd, uint32_t length,bool send)
{
    if (CameraGetData(cmd,send) < length)
    {
        LOG(LOG_LEVEL_ERROR, "Too less data received.");
        return false;
    }

    CameraLogGetCmd(cmd);
    return true;
}

bool Camera::CameraGetCmdSync()
{

    if (!CameraGetCmd(gsl::span<const uint8_t> (retcmd), CameraRetCmdLength,true))
    {
        LOG(LOG_LEVEL_ERROR, "Receiving data failed.");
        return false;
    }

    if (CameraParseSyncCmd(retcmd,CameraRetCmdLength) != CameraCmd::Sync)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid command received. Sync cmd was expected");
        return false;
    }

    return true;
}

bool Camera::CameraGetCmdData(CameraCmdData* cmdData)
{

    if (!CameraGetCmd(retcmd, -1,false))
    {
        LOG(LOG_LEVEL_ERROR, "Receiving data failed.");
        return false;
    }

    if (CameraParseDataCmd(retcmd, cmdData) != CameraCmd::Data)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid command received. Data cmd was expected. ");
        return false;
    }

    return true;
}

CameraCmd Camera::CameraGetCmdAck(uint8_t length)
{
    CameraCmdAck ackData;
    if (!CameraCmdAckInit(&ackData))
    {
        LOG(LOG_LEVEL_ERROR, "Initialiazing ack data failed.");
        return CameraCmd::Invalid;
    }

    if (!CameraGetCmd(retcmd,length,true))
    {
        LOG(LOG_LEVEL_ERROR, "Receiving data failed.");
        return CameraCmd::Invalid;
    }

    if (CameraParseAckCmd(retcmd, &ackData) != CameraCmd::Ack)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid command received. Ack cmd was expected.");
        return CameraCmd::Invalid;
    }

    return ackData.type;
}

bool Camera::CameraGetCmdAckSync()
{
    if (CameraGetCmdAck(CameraRetCmdLength) != CameraCmd::Sync)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid ack command received. Sync ack was expected.");
        return false;
    }
    return true;
}

bool Camera::CameraGetCmdAckInitial()
{
    if (CameraGetCmdAck(CameraCmdLength) != CameraCmd::Initial)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid ack command received. Initial ack was expected.");
        return false;
    }
    return true;
}

bool Camera::CameraGetCmdAckSnapshot()
{
    if (CameraGetCmdAck(CameraRetCmdLength) != CameraCmd::Snapshot)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid ack command received. Snapshot ack was expected.");
        return false;
    }
    return true;
}

bool Camera::CameraGetCmdAckGetPicture()
{
    if (CameraGetCmdAck(CameraRetCmdLength) != CameraCmd::GetPicture)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid ack command received. GetPicture ack was expected.");
        return false;
    }
    return true;
}

bool Camera::CameraGetCmdAckSetPackageSize()
{
    if (CameraGetCmdAck(CameraCmdLength) != CameraCmd::SetPackageSize)
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
    CameraSendCmd(gsl::span<const uint8_t>(cmd, CameraCmdLength));
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
    CameraSendCmd(gsl::span<const uint8_t>(cmd, CameraCmdLength));
}
void Camera::CameraSendCmdReset(void)
{
		cmd[0] = CameraCmdPrefix;
	    cmd[1]= (uint8_t)CameraCmd::Reset;
	    cmd[2]=0x00;
	    cmd[3]=0x00;
	    cmd[4]=0x00;
	    cmd[5]=(uint8_t)0xFF;

    LOG(LOG_LEVEL_INFO, "Sending Rest command.");
    CameraLogSendCmd(cmd);
    CameraSendCmd(gsl::span<const uint8_t> (cmd, CameraCmdLength));
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
    CameraSendCmd(gsl::span<const uint8_t> (cmd, CameraCmdLength));
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
    CameraSendCmd(gsl::span<const uint8_t> (cmd, CameraCmdLength));
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
    CameraSendCmd(gsl::span<const uint8_t> (cmd, CameraCmdLength));
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
    CameraSendCmd(gsl::span<const uint8_t> (cmd, CameraCmdLength));
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
    CameraSendCmd(gsl::span<const uint8_t> (cmd, CameraCmdLength));
    return true;
}


bool Camera::CameraSync()
{
    int8_t i = 60;


    while (i > 0)
    {
        CameraSendCmdSync();
        i--;

        if (!CameraGetCmdAckSync())
        {
            continue;
        }
        LOG(LOG_LEVEL_INFO, "Received Sync Ack Cmd.");
        if (!CameraGetCmdSync())
                      {
                          LOG(LOG_LEVEL_ERROR, "Receiving Sync command failed.");
                          return false;
                      }
                      LOG(LOG_LEVEL_INFO, "Received Sync Cmd.");

        Camera::CameraSendCmdAckSync();
        return true;
    }
    return false;
}

uint32_t Camera::CameraReceiveData(gsl::span<const uint8_t> data)
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

    if (cmdData.dataLength > (uint32_t) (data.length()))
    {
        //LOGF(LOG_LEVEL_ERROR, "Data buffer is to small. Passed:%d. Expected size:%d", dataLength, cmdData.dataLength);
        return 0;
    }

    LOG(LOG_LEVEL_INFO, "Start receiving data.");
    toLoad = cmdData.dataLength;
    while (toLoad > 0)
    {
        ret = CameraGetData(gsl::span<const uint8_t>(&data[cmdData.dataLength - toLoad], toLoad),true);
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

int8_t Camera::CameraReceiveJPEGData(gsl::span<const uint8_t>  data, uint16_t packageSize)
{
    uint16_t ret = 0;
    uint8_t i = 0;

    if (packageSize > 512 || packageSize < 64)
    {
        LOG(LOG_LEVEL_ERROR, "Package size is invalid. It should be smaller than 512 and larger than 64.");
        return false;
    }

    uint8_t packageCnt = data.length() / (packageSize - 6) + (data.length() % (packageSize - 6) != 0 ? 1 : 0);
    for (i = 0; i < packageCnt; i++)
    {
            ret = CameraGetData(gsl::span<const uint8_t>(&data[i*packageSize], packageSize),true);
            if (ret > 0)
            {
                CameraSendCmdAck(CameraCmd::None,i & 0xff,(uint8_t)(i >> 8));
            }
    }
    return i;
}

