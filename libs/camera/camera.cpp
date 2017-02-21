#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "leuart/leuart.h"
#include "logger/logger.h"
#include "system.h"


#include "camera.h"
#include "uart/Uart.h"


using namespace devices::camera;
using namespace std::chrono_literals;


Camera::Camera(drivers::uart::Uart uartBus) : _uartBus(uartBus)
{
}

bool Camera::InitializeJPEGPicture(CameraJPEGResolution resolution) {



	    	if (!CameraSync())
	    	{
	    		LOG(LOG_LEVEL_ERROR, "---------------- Sync failed -------------------------\n");
	    		isInitialized = false;
	    		return false;
	    	}


	        CameraSendCmdJPEGInitial(resolution);


	        if (!CameraGetCmdAckInitial())
	        {
	            LOG(LOG_LEVEL_ERROR, "---------------- Ack Initial failed ------------------\n");
	            isInitialized = false;
	            return false;
	        }


	        if (!CameraSendCmdSetPackageSize(512))
	        {
	            LOG(LOG_LEVEL_ERROR, "---------------- Send Cmd Set Package failed ---------\n");
	            isInitialized = false;
	            return false;
	        }

	        if (!CameraGetCmdAckSetPackageSize())
	        {
	            LOG(LOG_LEVEL_ERROR, "---------------- ACK Set Package Size failed ---------\n");
	            isInitialized = false;
	            return false;
	        }

	        CameraSendCmdSnapshot(CameraSnapshotType::Compressed);

	        if (!CameraGetCmdAckSnapshot())
	        {
	            LOG(LOG_LEVEL_ERROR, "---------------- ACK Snapshot failed -----------------\n");
	            isInitialized = false;
	            return false;
	        }
	        isInitialized = true;
	        return true;
}




int32_t Camera::CameraGetJPEGPicture(uint8_t* data, uint32_t dataLength, bool reset)
{
	UNREFERENCED_PARAMETER(data);
	UNREFERENCED_PARAMETER(dataLength);
	uint32_t ret = 0;

	if(!isInitialized)
			{
				return 0;
			}


        CameraSendCmdGetPicture(CameraPictureType::Snapshot);

        if (!CameraGetCmdAckGetPicture())
        {
            LOG(LOG_LEVEL_ERROR, "---------------- ACK GetSnapshot failed ---------------\n");

            return 0;

        }

        CameraCmdData cmdData;
        if (!CameraCmdDataInit(&cmdData))
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Init Cmd Data struct failed ---------\n");
            return 0;
        }

        if (!CameraGetCmdData(&cmdData))
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Get Cmd Data failed -----------------\n");
            return 0;
        }

        if (CameraPictureType::Snapshot != cmdData.type || cmdData.dataLength <= 0)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Invalid Cmd Data received -----------\n");
            return 0;
        }
        LOG(LOG_LEVEL_ERROR, "---------------- Cmd Data received ----------------\n");

        if(cmdData.dataLength > dataLength)
        {
        	LOG(LOG_LEVEL_ERROR, "---------------- Invalid input buffer size ----------------\n");
        	return 0;
        }
        ret = CameraReceiveJPEGData(data, cmdData.dataLength,512);
        if (ret == 0)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Invalid Data command ----------------\n");
            return 0;
        }
        if(reset)
        {
        	isInitialized=false;
        	CameraSendCmdReset();
        }
        return ret;

}
bool Camera::InitializeRAWPicture(CameraRAWImageFormat format, CameraRAWResolution resolution){

        if (!CameraSync())
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Sync failed -------------------------\n");
            isInitialized = false;
            return false;
        }

        CameraSendCmdRAWInitial(format, resolution);

        if (!CameraGetCmdAckInitial())
        {
            LOG(LOG_LEVEL_ERROR, "---------------- ACK Initial failed ------------------\n");
            isInitialized = false;
            return false;
        }

        CameraSendCmdSnapshot(CameraSnapshotType::Uncompressed);

        if (!CameraGetCmdAckSnapshot())
        {
            LOG(LOG_LEVEL_ERROR, "---------------- ACK Snapshot failed -----------------\n");
            isInitialized = false;
            return false;
        }
        isInitialized = true;
        return true;
    }





int32_t Camera::CameraGetRAWPicture(uint8_t* data,
                            uint32_t dataLength,bool reset)
{

	uint32_t imageLength = 0;
	uint32_t ret = 0;
		if(!isInitialized)
		{
			return 0;
		}


        CameraSendCmdGetPicture(CameraPictureType::RAW);

        if (!CameraGetCmdAckGetPicture())
        {
            LOG(LOG_LEVEL_ERROR, "---------------- ACK GetPicture failed----------------\n");
            return 0;
        }

        imageLength = CameraGetRAWDataLength(CameraRAWImageFormat::RGB565, CameraRAWResolution::_160x120);
        if (imageLength == 0)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Get Data Length failed --------------\n");
            return 0;
        }

        ret = CameraReceiveData(data, dataLength);
        if (ret == 0)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Invalid Data command ----------------\n");
            return 0;
        }

        CameraSendCmdAckData();
        if(reset)
        {
        isInitialized=false;
        CameraSendCmdReset();
        }

    return ret;
}
