#include <algorithm>
#include "base/reader.h"

#include "uplink.h"

using std::uint8_t;
using std::copy;
using gsl::span;

using namespace telecommands;
using telecommands::handling::DecodeTelecommandFailureReason;
using telecommands::handling::DecodeTelecommandResult;

UplinkProtocol::UplinkProtocol(std::uint32_t securityCode) : _securityCode(securityCode)
{
}

DecodeTelecommandResult UplinkProtocol::Decode(span<const uint8_t> frame)
{
    if (frame.size() < 5)
    {
        return DecodeTelecommandResult::Failure(DecodeTelecommandFailureReason::MalformedFrame);
    }

    Reader r(frame);

    auto code = r.ReadDoubleWordBE();

    if (code != this->_securityCode)
    {
        return DecodeTelecommandResult::Failure(DecodeTelecommandFailureReason::InvalidSecurityCode);
    }

    auto command = r.ReadByte();
    auto parameters = r.Remaining();

    return DecodeTelecommandResult::Success(command, parameters);
}
