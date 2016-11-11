#include "obc.h"
#include "io_map.h"

using namespace std;

OBC::OBC()
    : I2C(&I2CBuses[I2C_SYSTEM_BUS].ErrorHandling.Base, &I2CBuses[I2C_PAYLOAD_BUS].ErrorHandling.Base), //
      Telecommands(comm),                                                                               //
      AllTelecommands{&Telecommands.Ping},                                                              //
      FrameHandler(FrameUnpacker, FrameUnpacker, AllTelecommands, COUNT_OF(AllTelecommands)),           //
      comm(*I2C.System, FrameHandler)
{
}

TeleCommandDecryptStatus TelecommandFrameUnpacker::Decrypt(
    gsl::span<const uint8_t> frame, gsl::span<uint8_t> decrypted, size_t& decryptedDataLength)
{
    auto lastCopied = std::copy(frame.cbegin(), frame.cend(), decrypted.begin());

    decryptedDataLength = lastCopied - decrypted.begin();

    return TeleCommandDecryptStatus::OK;
}

PingTelecommand::PingTelecommand(devices::comm::CommObject& comm) : _comm(comm)
{
}

TeleCommandDecodeFrameStatus TelecommandFrameUnpacker::Decode(
    gsl::span<const uint8_t> frame, uint8_t& commandCode, gsl::span<const uint8_t>& parameters)
{
    if (frame.length() < 1)
    {
        return TeleCommandDecodeFrameStatus::Failed;
    }

    commandCode = frame[0];
    parameters = frame.subspan(1, frame.length() - 1);

    return TeleCommandDecodeFrameStatus::OK;
}

void PingTelecommand::Handle(gsl::span<const uint8_t> parameters)
{
    UNREFERENCED_PARAMETER(parameters);

    const char* response = "PONG";

    this->_comm.SendFrame(gsl::span<const uint8_t>(reinterpret_cast<const uint8_t*>(response), 4));
}

std::uint8_t PingTelecommand::CommandCode() const
{
    return static_cast<uint8_t>('P');
}

TelecommandsObject::TelecommandsObject(devices::comm::CommObject& comm) : Ping(comm)
{
}
