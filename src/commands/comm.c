#include <string.h>

#include "commands.h"

#include "devices/comm.h"
#include "system.h"

void SendFrameHandler(uint16_t argc, char* argv[])
{
	UNREFERENCED_PARAMETER(argc);

	uint8_t len = strlen(argv[0]);

	CommSendFrame(argv[0], len);
}
