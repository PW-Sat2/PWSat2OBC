#ifndef LIBS_COMM_HANDLING_INCLUDE_COMM_HANDLING_COMM_HANDLING_H_
#define LIBS_COMM_HANDLING_INCLUDE_COMM_HANDLING_COMM_HANDLING_H_

#include <cstdint>
#include <gsl/span>
#include "comm/comm.h"
#include "system.h"

struct IDecryptFrame
{
    virtual std::size_t Decrypt(gsl::span<const uint8_t> frame, gsl::span<uint8_t> decrypted) = 0;
};

struct IDecodeTelecommand
{
    virtual void Decode(gsl::span<const std::uint8_t> frame, std::uint8_t& commandCode, gsl::span<const std::uint8_t>& parameters) = 0;
};

struct IHandleTeleCommand
{
    virtual void Handle(gsl::span<const std::uint8_t> parameters) = 0;
    virtual std::uint8_t CommandCode() const = 0;
};

class IncomingTelecommandHandler : public devices::comm::IHandleFrame
{
  public:
    IncomingTelecommandHandler(IDecryptFrame& decryptFrame,
        IDecodeTelecommand& decodeTelecommand,
        IHandleTeleCommand** telecommands,
        std::size_t telecommandsCount);

    virtual void HandleFrame(devices::comm::CommFrame& frame) override;

  private:
    IDecryptFrame& _decryptFrame;
    IDecodeTelecommand& _decodeTelecommand;
    IHandleTeleCommand** _telecommands;
    std::size_t _telecommandsCount;

    void DispatchCommandHandler(std::uint8_t commandCode, gsl::span<const uint8_t> parameters);
};

#endif /* LIBS_COMM_HANDLING_INCLUDE_COMM_HANDLING_COMM_HANDLING_H_ */
