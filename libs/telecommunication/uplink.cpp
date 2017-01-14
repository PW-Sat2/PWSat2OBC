#include "uplink.h"
#include <algorithm>
#include "base/reader.h"

using std::uint8_t;
using std::copy;
using gsl::span;

using namespace telecommunication::uplink;

UplinkProtocol::UplinkProtocol(std::uint32_t securityCode) : _securityCode(securityCode)
{
}

DecodeTelecommandResult UplinkProtocol::Decode(span<const uint8_t> frame)
{
    Reader r(frame);

    auto code = r.ReadDoubleWordBE();
    auto command = r.ReadByte();
    auto parameters = r.ReadToEnd();

    if (!r.Status())
    {
        return DecodeTelecommandResult::Failure(DecodeTelecommandFailureReason::MalformedFrame);
    }

    if (code != this->_securityCode)
    {
        return DecodeTelecommandResult::Failure(DecodeTelecommandFailureReason::InvalidSecurityCode);
    }

    return DecodeTelecommandResult::Success(command, parameters);
}
