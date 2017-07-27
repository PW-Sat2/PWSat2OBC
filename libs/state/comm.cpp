#include <cstring>
#include "base/reader.h"
#include "base/writer.h"
#include "comm/MessageState.hpp"

using namespace std::chrono_literals;

namespace state
{
    static const char* DefaultMessage =
        "We are the Borg. Lower your shields and surrender your ships. We will add your biological and "
        "technological distinctiveness to our own. Your culture will adapt to service us. Resistance is futile.";

    MessageState::MessageState() : _interval(5min), _repeatCount(3)
    {
        this->_message.fill(0);
        strcpy(reinterpret_cast<char*>(this->_message.data()), DefaultMessage);
    }

    MessageState::MessageState(std::chrono::minutes interval, std::uint8_t repeatCount, gsl::span<const std::uint8_t> message)
        : _interval(interval), _repeatCount(repeatCount)
    {
        this->_message.fill(0);
        std::copy(message.begin(), message.end(), this->_message.begin());
    }

    std::chrono::minutes MessageState::Interval() const
    {
        return this->_interval;
    }

    std::uint8_t MessageState::RepeatCount() const
    {
        return this->_repeatCount;
    }

    gsl::span<const std::uint8_t> MessageState::Message() const
    {
        return this->_message;
    }

    void MessageState::Write(Writer& writer) const
    {
        writer.WriteByte(static_cast<std::uint8_t>(this->_interval.count()));
        writer.WriteByte(this->_repeatCount);
        writer.WriteArray(this->_message);
    }

    void MessageState::Read(Reader& reader)
    {
        this->_interval = std::chrono::minutes(reader.ReadByte());
        this->_repeatCount = reader.ReadByte();
        auto r = reader.ReadArray(this->_message.size());
        std::copy(r.begin(), r.end(), this->_message.begin());
    }
}
