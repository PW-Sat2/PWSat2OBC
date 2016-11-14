#ifndef LIBS_TELECOMMAND_HANDLING_INCLUDE_TELECOMMAND_HANDLING_TELECOMMAND_HANDLING_H_
#define LIBS_TELECOMMAND_HANDLING_INCLUDE_TELECOMMAND_HANDLING_TELECOMMAND_HANDLING_H_

#include <cstdint>
#include <gsl/span>
#include "comm/comm.h"
#include "system.h"

enum class TeleCommandDecryptStatus
{
    OK = 0,
    Failed = 1
};

enum class TeleCommandDecodeFrameStatus
{
    OK,
    Failed
};

struct IDecryptFrame
{
    virtual TeleCommandDecryptStatus Decrypt(
        gsl::span<const uint8_t> frame, gsl::span<uint8_t> decrypted, std::size_t& decryptedDataLength) = 0;
};

struct IDecodeTelecommand
{
    virtual TeleCommandDecodeFrameStatus Decode(
        gsl::span<const std::uint8_t> frame, std::uint8_t& commandCode, gsl::span<const std::uint8_t>& parameters) = 0;
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

#endif /* LIBS_TELECOMMAND_HANDLING_INCLUDE_TELECOMMAND_HANDLING_TELECOMMAND_HANDLING_H_ */
