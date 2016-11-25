#include "uplink.h"
#include <algorithm>

using std::uint8_t;
using std::copy;
using gsl::span;

using namespace telecommands;
using telecommands::handling::DecryptFrameResult;
using telecommands::handling::DecodeTelecommandFailureReason;
using telecommands::handling::DecodeTelecommandResult;

DecryptFrameResult UplinkProtocol::Decrypt(span<const uint8_t> frame, span<uint8_t> decrypted)
{
    auto lastCopied = copy(frame.cbegin(), frame.cend(), decrypted.begin());

    auto decryptedDataLength = lastCopied - decrypted.begin();

    return DecryptFrameResult::Success(decrypted.subspan(0, decryptedDataLength));
}

DecodeTelecommandResult UplinkProtocol::Decode(span<const uint8_t> frame)
{
    if (frame.length() < 1)
    {
        return DecodeTelecommandResult::Failure(DecodeTelecommandFailureReason::GeneralError);
    }

    return DecodeTelecommandResult::Success(frame[0], frame.subspan(1, frame.length() - 1));
}
