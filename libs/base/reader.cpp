#include "reader.h"
#include <utility>

Reader::Reader() : position(0), isValid(false)
{
}

Reader::Reader(gsl::span<const uint8_t> view)
{
    Initialize(std::move(view));
}

bool Reader::UpdateState(uint16_t requestedSize)
{
    if (this->isValid)
    {
        this->position += requestedSize;
        this->isValid = this->position <= this->buffer.size();
    }

    return this->isValid;
}

void Reader::Initialize(gsl::span<const uint8_t> view)
{
    this->buffer = view;
    Reset();
}

bool Reader::Skip(uint16_t length)
{
    return UpdateState(length);
}

uint8_t Reader::ReadByte()
{
    if (!UpdateState(1))
    {
        return 0;
    }
    else
    {
        return this->buffer[this->position - 1];
    }
}

uint16_t Reader::ReadWordLE()
{
    if (!UpdateState(2))
    {
        return 0;
    }
    else
    {
        uint16_t value = this->buffer[this->position - 1];
        value <<= 8;
        value += this->buffer[this->position - 2];
        return value;
    }
}

uint16_t Reader::ReadWordBE()
{
    if (!UpdateState(2))
    {
        return 0;
    }
    else
    {
        uint16_t value = this->buffer[this->position - 2];
        value <<= 8;
        value += this->buffer[this->position - 1];
        return value;
    }
}

uint32_t Reader::ReadDoubleWordLE()
{
    if (!UpdateState(4))
    {
        return 0;
    }
    else
    {
        uint32_t value = this->buffer[this->position - 1];
        value <<= 8;
        value += this->buffer[this->position - 2];
        value <<= 8;
        value += this->buffer[this->position - 3];
        value <<= 8;
        value += this->buffer[this->position - 4];
        return value;
    }
}

uint32_t Reader::ReadDoubleWordBE()
{
    if (!UpdateState(4))
    {
        return 0;
    }
    else
    {
        uint32_t value = this->buffer[this->position - 4];
        value <<= 8;
        value += this->buffer[this->position - 3];
        value <<= 8;
        value += this->buffer[this->position - 2];
        value <<= 8;
        value += this->buffer[this->position - 1];
        return value;
    }
}

uint64_t Reader::ReadQuadWordLE()
{
    if (!UpdateState(8))
    {
        return 0;
    }
    else
    {
        uint64_t value = this->buffer[this->position - 1];
        value <<= 8;
        value += this->buffer[this->position - 2];
        value <<= 8;
        value += this->buffer[this->position - 3];
        value <<= 8;
        value += this->buffer[this->position - 4];
        value <<= 8;
        value += this->buffer[this->position - 5];
        value <<= 8;
        value += this->buffer[this->position - 6];
        value <<= 8;
        value += this->buffer[this->position - 7];
        value <<= 8;
        value += this->buffer[this->position - 8];
        return value;
    }
}

gsl::span<const uint8_t> Reader::ReadArray(uint16_t length)
{
    if (!UpdateState(length))
    {
        return gsl::span<const std::uint8_t>();
    }
    else
    {
        return this->buffer.subspan(this->position - length, length);
    }
}

int32_t Reader::RemainingSize()
{
    return this->buffer.size() - this->position;
}

gsl::span<const uint8_t> Reader::ReadToEnd()
{
    return this->ReadArray(this->RemainingSize());
}
