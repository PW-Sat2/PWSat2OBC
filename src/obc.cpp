#include "obc.h"

using namespace std;
using telecommands::handling::DecodeFrameStatus;
using telecommands::handling::DecryptStatus;
using telecommands::handling::IHandleTeleCommand;

OBC::OBC()
    : I2C(&I2CBuses[0].ErrorHandling.Base, &I2CBuses[1].ErrorHandling.Base), //
      Communication(*I2C.System)
{
}

void OBC::Initialize()
{
    this->Communication.Initialize();
}
