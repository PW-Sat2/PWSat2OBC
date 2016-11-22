#include "uplink.h"

using namespace telecommands;
using telecommands::handling::DecryptStatus;
using telecommands::handling::DecodeFrameStatus;
using telecommands::handling::IHandleTeleCommand;

DecryptStatus UplinkProtocol::Decrypt(gsl::span<const uint8_t> frame, gsl::span<uint8_t> decrypted, size_t& decryptedDataLength)
{
    auto lastCopied = std::copy(frame.cbegin(), frame.cend(), decrypted.begin());

    decryptedDataLength = lastCopied - decrypted.begin();

    return DecryptStatus::Success;
}

DecodeFrameStatus UplinkProtocol::Decode(gsl::span<const uint8_t> frame, uint8_t& commandCode, gsl::span<const uint8_t>& parameters)
{
    if (frame.length() < 1)
    {
        return DecodeFrameStatus::Failed;
    }

    commandCode = frame[0];
    parameters = frame.subspan(1, frame.length() - 1);

    return DecodeFrameStatus::Success;
}
