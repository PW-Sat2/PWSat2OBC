#include "commands.h"
#include "obc.h"

void WaitForOBCInitialization(std::uint16_t /*argc*/, char* /*argv*/ [])
{
    Main.StateFlags.WaitAny(OBC::InitializationFinishedFlag, false, InfiniteTimeout);
    Main.terminal.Puts("Initialized");
}
