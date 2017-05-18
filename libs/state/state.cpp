#include "struct.h"

using namespace std::chrono_literals;

SystemState::SystemState()                //
    : Time(0ms),                          //
      SailOpened(false),                  //
      AdcsMode(adcs::AdcsMode::Disabled), //
      PersistentState()                   //
{
}
