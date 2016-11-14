#ifndef LIBS_COMM_HANDLING_INCLUDE_COMM_HANDLING_COMM_HANDLING_H_
#define LIBS_COMM_HANDLING_INCLUDE_COMM_HANDLING_COMM_HANDLING_H_

#include "comm/comm.h"
#include "system.h"

class IncomingTelecommandHandler : public drivers::devices::comm::IHandleFrame
{
  public:
    virtual void HandleFrame(drivers::devices::comm::CommObject& comm, drivers::devices::comm::CommFrame& frame) override;
};

#endif /* LIBS_COMM_HANDLING_INCLUDE_COMM_HANDLING_COMM_HANDLING_H_ */
