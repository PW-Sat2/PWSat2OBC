#include "communication.h"
#include "gsl/span"
#include "logger/logger.h"
#include "settings.h"
#include "telecommunication/downlink.h"

using drivers::i2c::II2CBus;
using gsl::span;
using std::uint8_t;
using telecommunication::uplink::IHandleTeleCommand;

using namespace obc;
using namespace obc::telecommands;

OBCCommunication::OBCCommunication(
    devices::comm::CommObject& commDriver,
    IHasState<SystemState>& stateContainer,
    boot::BootSettings& bootSettings,
    services::power::IPowerControl& powerControl,
    drivers::i2c::II2CBus& systemBus,
    drivers::i2c::II2CBus& payload)
    : Comm(commDriver),                                                                                                               //
      UplinkProtocolDecoder(settings::CommSecurityCode),                                                                              //
      SupportedTelecommands(                                                                                                          //
          PingTelecommand(),                                                                                                          //
          SetBootSlotsTelecommand(bootSettings),                                                                                      //
          PowerCycle(powerControl),                                                                                                   //
          RawI2CTelecommand(systemBus, payload),                                                                                      //
          SendPeriodicMessageTelecommand(stateContainer),                                                               //
          SetBitrateTelecommand(),                                                                                      //
          obc::telecommands::ReadMemoryTelecommand() //
          ),                                         //
      TelecommandHandler(UplinkProtocolDecoder, SupportedTelecommands.Get())
{
}

void OBCCommunication::InitializeRunlevel1()
{
    this->Comm.SetFrameHandler(this->TelecommandHandler);
    if (!this->Comm.RestartHardware())
    {
        LOG(LOG_LEVEL_ERROR, "Unable to restart COMM hardware");
    }
}

void OBCCommunication::InitializeRunlevel2()
{
    if (!this->Comm.StartTask())
    {
        LOG(LOG_LEVEL_ERROR, "Unable to start comm task");
    }
}
