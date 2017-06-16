#include "base/BitWriter.hpp"
#include <algorithm>
#include <cstring>
#include <limits>
#include "system.h"

static constexpr std::uint8_t BitsPerByte = std::numeric_limits<std::uint8_t>::digits;
static constexpr std::uint8_t BitsPerWord = std::numeric_limits<std::uint16_t>::digits;
static constexpr std::uint8_t BitsPerDWord = std::numeric_limits<std::uint32_t>::digits;
static constexpr std::uint8_t BitsPerQWord = std::numeric_limits<std::uint64_t>::digits;

static const std::uint16_t WordMask[] = {
    0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff, 0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff //
};

static_assert((BitsPerWord + 1) <= count_of(WordMask), "Extend Mask array");

BitWriter::BitWriter()
    : _bitPosition(0),  //
      _bytePosition(0), //
      _bitLimit(0),     //
      _isValid(false)
{
}

BitWriter::BitWriter(gsl::span<std::uint8_t> view)
{
    Initialize(std::move(view));
}

inline void BitWriter::Reset()
{
    this->_bitPosition = 0;
    this->_bytePosition = 0;
    this->_bitLimit = this->_buffer.length() * BitsPerByte;
    this->_isValid = this->_buffer.length() > 0;
}

static inline std::uint32_t BitsToBytes(std::uint32_t bits)
{
    return (bits + BitsPerByte - 1) / BitsPerByte;
}

std::uint32_t BitWriter::GetBitDataLength() const
{
    return this->_bitPosition + this->_bytePosition * BitsPerByte;
}

std::uint32_t BitWriter::GetByteDataLength() const
{
    return BitsToBytes(this->_bitPosition) + this->_bytePosition;
}

inline bool BitWriter::UpdateStatus(std::uint32_t length, std::uint32_t lengthLimit)
{
    return (this->_isValid = this->_isValid && //
            (length <= lengthLimit) &&         //
            ((GetBitDataLength() + length) <= this->_bitLimit));
}

bool BitWriter::Write(std::uint8_t value)
{
    return WriteWord(value, BitsPerByte);
}

bool BitWriter::Write(std::uint16_t value)
{
    return WriteWord(value, BitsPerWord);
}

bool BitWriter::Write(std::uint32_t value)
{
    return WriteDoubleWord(value, BitsPerDWord);
}

bool BitWriter::Write(std::uint64_t value)
{
    return WriteQuadWord(value, BitsPerQWord);
}

bool BitWriter::WriteSpan(gsl::span<const std::uint8_t> buffer)
{
    const auto size = buffer.size() * BitsPerByte;
    if (!UpdateStatus(size, size))
    {
        return false;
    }

    if (this->_bitPosition == 0)
    {
        std::memcpy(this->_buffer.data() + this->_bytePosition, buffer.data(), buffer.size());
        this->_bytePosition += buffer.size();
    }
    else
    {
        for (auto byte : buffer)
        {
            WriteWord(byte, this->_buffer.data() + this->_bytePosition, BitsPerByte);
        }
    }

    return true;
}

void BitWriter::WriteWord(std::uint16_t value, std::uint8_t* position, std::uint8_t length)
{
    const std::uint32_t combined = (static_cast<std::uint32_t>(value & WordMask[length]) << this->_bitPosition) + //
        (*position & WordMask[this->_bitPosition]);

    const std::uint32_t bits = length + this->_bitPosition;
    const std::uint32_t bytes = BitsToBytes(bits);
    *position++ = combined;
    if (bytes > 1)
    {
        *position++ = combined >> BitsPerByte;
        if (bytes > 2)
        {
            *position++ = combined >> (2 * BitsPerByte);
        }
    }

    this->_bytePosition += bits / BitsPerByte;
    this->_bitPosition = bits & (BitsPerByte - 1);
}

bool BitWriter::WriteWord(std::uint16_t value, std::uint8_t length)
{
    if (!UpdateStatus(length, BitsPerWord))
    {
        return false;
    }

    if (length > 0)
    {
        WriteWord(value, this->_buffer.data() + this->_bytePosition, std::min(length, BitsPerWord));
    }

    return true;
}

bool BitWriter::WriteDoubleWord(std::uint32_t value, std::uint8_t length)
{
    if (!UpdateStatus(length, BitsPerDWord))
    {
        return false;
    }

    if (length > 0)
    {
        WriteWord(value, this->_buffer.data() + this->_bytePosition, std::min(length, BitsPerWord));
        if (length > BitsPerWord)
        {
            WriteWord(value >> BitsPerWord, this->_buffer.data() + this->_bytePosition, length - BitsPerWord);
        }
    }

    return true;
}

bool BitWriter::WriteQuadWord(std::uint64_t value, std::uint8_t length)
{
    if (!UpdateStatus(length, BitsPerQWord))
    {
        return false;
    }

    if (length > 0)
    {
        WriteWord(value, this->_buffer.data() + this->_bytePosition, std::min(length, BitsPerWord));
        if (length > BitsPerWord)
        {
            WriteWord(value >> BitsPerWord,
                this->_buffer.data() + this->_bytePosition,
                std::min<std::uint8_t>(length - BitsPerWord, BitsPerWord));
            if (length > (2 * BitsPerWord))
            {
                WriteWord(value >> (2 * BitsPerWord),
                    this->_buffer.data() + this->_bytePosition,
                    std::min<std::uint8_t>(length - (2 * BitsPerWord), BitsPerWord));
                if (length > (3 * BitsPerWord))
                {
                    WriteWord(value >> (3 * BitsPerWord), this->_buffer.data() + this->_bytePosition, length - 3 * BitsPerWord);
                }
            }
        }
    }

    return true;
}
