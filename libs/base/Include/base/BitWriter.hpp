#ifndef LIBS_BASE_BIT_WRITER_HPP
#define LIBS_BASE_BIT_WRITER_HPP

#pragma once

#include <bitset>
#include <cstdint>
#include <type_traits>
#include "fwd.hpp"
#include "gsl/span"
#include "system.h"

/**
 * @brief Buffer bit writer
 *
 * This object is supposed to provide means of writing values of varying bit length to specified buffer
 * in a way that is independent of both: current memory layout and the actual buffer location in memory.
 */
class BitWriter
{
  public:
    /**
     * @brief Default .ctor
     */
    BitWriter();

    /**
     * @brief Initializes generic buffer writer.
     *
     * @param[in] view Window into memory buffer to which the data is written.
     */
    BitWriter(gsl::span<std::uint8_t> view);

    /**
     * @brief Initializes generic buffer writer.
     *
     * @param[in] view Window into memory buffer to which the data is written.
     */
    void Initialize(gsl::span<std::uint8_t> view);

    /**
     * @brief Returns current writer status.
     * @retval true Buffer is valid and there is still some space left in it.
     * @retval false An attempt to write data beyond the buffer end has been made.
     */
    bool Status() const;

    /**
     * @brief Returns the number of bits already written to the buffer.
     * @return Number of bits already written to the buffer.
     */
    std::uint32_t GetBitDataLength() const;

    /**
     * @brief Returns the number of occupied bits in last used buffer byte.
     * @return Number of of occupied bits in last used buffer byte.
     */
    std::uint32_t GetBitFraction() const;

    /**
     * @brief Returns the number of bytes already written to the buffer.
     *
     * @remark The last partially used byte is also included in the value returned by this method.
     * @return Number of bytes already written to the buffer.
     */
    std::uint32_t GetByteDataLength() const;

    /**
     * @brief Appends n-bit value to the buffer and moves the current position to the next free bit.
     * @param[in] value Value that should be added to writer output.
     * @param[in] length Size of the value in bits.
     *
     * @remark Only the lowest length bits will be written to writer output, if the value contains number
     * that is longer than provided length it will be truncated.
     * @return Operation status.
     */
    bool WriteWord(std::uint16_t value, std::uint8_t length);

    /**
     * @brief Appends n-bit value to the buffer and moves the current position to the next free bit.
     * @param[in] value Value that should be added to writer output.
     * @param[in] length Size of the value in bits.
     *
     * @remark Only the lowest length bits will be written to writer output, if the value contains number
     * that is longer than provided length it will be truncated.
     * @return Operation status.
     */
    bool WriteDoubleWord(std::uint32_t value, std::uint8_t length);

    /**
     * @brief Appends n-bit value to the buffer and moves the current position to the next free bit.
     * @param[in] value Value that should be added to writer output.
     * @param[in] length Size of the value in bits.
     *
     * @remark Only the lowest length bits will be written to writer output, if the value contains number
     * that is longer than provided length it will be truncated.
     * @return Operation status.
     */
    bool WriteQuadWord(std::uint64_t value, std::uint8_t length);

    /**
     * @brief Appends array of bytes to the writer output.
     * @param[in] buffer Array of bytes that should be added to writer output.
     * @return Operation status.
     */
    bool WriteSpan(gsl::span<const std::uint8_t> buffer);

    /**
     * @brief Appends 8-bit value to the buffer and moves the current position to the next free bit.
     * @param[in] value Value that should be added to writer output.
     * @return Operation status.
     */
    bool Write(std::uint8_t value);

    /**
     * @brief Appends 16-bit value to the buffer and moves the current position to the next free bit.
     * @param[in] value Value that should be added to writer output.
     * @return Operation status.
     */
    bool Write(std::uint16_t value);

    /**
     * @brief Appends 32-bit value to the buffer and moves the current position to the next free bit.
     * @param[in] value Value that should be added to writer output.
     * @return Operation status.
     */
    bool Write(std::uint32_t value);

    /**
     * @brief Appends 64-bit value to the buffer and moves the current position to the next free bit.
     * @param[in] value Value that should be added to writer output.
     * @return Operation status.
     */
    bool Write(std::uint64_t value);

    /**
     * @brief Appends integer value to the buffer and moves the current position to the next free bit.
     * @param[in] value Value that should be added to writer output.
     * @return Operation status.
     */
    std::enable_if<!std::is_same<unsigned int, std::uint8_t>::value && //
            !std::is_same<unsigned int, std::uint16_t>::value &&       //
            !std::is_same<unsigned int, std::uint32_t>::value &&       //
            !std::is_same<unsigned int, std::uint64_t>::value,         //
        bool>::type
        Write(unsigned int value);

    /**
     * @brief Appends boolean to the buffer and moves the current position to the next free bit.
     * @param[in] value Value that should be added to writer output.
     * @return Operation status.
     */
    bool Write(bool value);

    /**
     * @brief Appends n-bit value to the buffer and moves the current position to the next free bit.
     * @param[in] value Value that should be added to writer output.
     * @return Operation status.
     */
    template <typename Underlying, std::uint8_t BitsCount> bool Write(const BitValue<Underlying, BitsCount>& value);

    /**
     * @brief Appends list values to the buffer and moves the current position to the next free bit.
     * @param[in] list List of values that should be added to writer output.
     * @return Operation status.
     */
    template <typename T> bool Write(gsl::span<T> list);

    /**
     * @brief Appends list values to the buffer and moves the current position to the next free bit.
     * @param[in] list List of values that should be added to writer output.
     * @return Operation status.
     */
    template <typename T, size_t N> bool Write(const std::array<T, N>& list);

    /**
     * @brief Appends generic value to the buffer and moves the current position to the next free bit.
     * @param[in] value Value that should be added to writer output.
     * @return Operation status.
     * @remark Overload for enumerations
     */
    template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type = 0> bool Write(T value);

    /**
     * @brief Appends generic value to the buffer and moves the current position to the next free bit.
     * @param[in] value Value that should be added to writer output.
     * @return Operation status.
     * @remark Overload for fundamental types
     */
    template <typename T, typename std::enable_if<std::is_fundamental<T>::value, int>::type = 0> bool Write(T value);

    /**
     * @brief Appends all bits from bitset to buffer
     * @param value Bitset to append
     * @return Operation status
     */
    template <std::size_t Size> bool Write(const std::bitset<Size>& value);

    /**
     * @brief Returns view for used part of buffer
     * @return Span covering used part of buffer including last partially used byte.
     */
    gsl::span<std::uint8_t> Capture();

    /**
     * @brief Resets reader to the initial state.
     */
    void Reset();

  private:
    void WriteWord(std::uint16_t value, std::uint8_t* position, std::uint8_t length);

    /**
     * @brief Appends n-bit value to the buffer and moves the current position to the next free bit.
     * @param[in] value Value that should be added to writer output.
     * @param[in] length Size of the value in bits.
     *
     * @remark Only the lowest length bits will be written to writer output, if the value contains number
     * that is longer than provided length it will be truncated.
     * @return Operation status.
     */
    bool Write(std::uint8_t value, std::uint8_t length);

    /**
     * @brief Appends n-bit value to the buffer and moves the current position to the next free bit.
     * @param[in] value Value that should be added to writer output.
     * @param[in] length Size of the value in bits.
     *
     * @remark Only the lowest length bits will be written to writer output, if the value contains number
     * that is longer than provided length it will be truncated.
     * @return Operation status.
     */
    bool Write(std::uint16_t value, std::uint8_t length);

    /**
     * @brief Appends n-bit value to the buffer and moves the current position to the next free bit.
     * @param[in] value Value that should be added to writer output.
     * @param[in] length Size of the value in bits.
     *
     * @remark Only the lowest length bits will be written to writer output, if the value contains number
     * that is longer than provided length it will be truncated.
     * @return Operation status.
     */
    bool Write(std::uint32_t value, std::uint8_t length);

    /**
     * @brief Appends n-bit value to the buffer and moves the current position to the next free bit.
     * @param[in] value Value that should be added to writer output.
     * @param[in] length Size of the value in bits.
     *
     * @remark Only the lowest length bits will be written to writer output, if the value contains number
     * that is longer than provided length it will be truncated.
     * @return Operation status.
     */
    bool Write(std::uint64_t value, std::uint8_t length);

    bool UpdateStatus(std::uint32_t length, std::uint32_t lengthLimit);

    /**
     * @brief Pointer to the buffer in memory.
     */
    gsl::span<std::uint8_t> _buffer;

    /**
     * @brief Number of bits used in last used buffer byte.
     *
     * This value points to the first not yet processed bit.
     */
    std::uint32_t _bitPosition;

    /**
     * @brief Current buffer location.
     *
     * This value points to the first not yet fully processed byte.
     */
    std::uint32_t _bytePosition;

    /**
     * @brief _buffer size in bits.
     */
    std::uint32_t _bitLimit;

    /**
     * @brief Current buffer status.
     *
     *  - True -> Data is valid.
     *  - False -> Buffer overflow detected.
     */
    bool _isValid;
};

template <typename Underlying, std::uint8_t BitsCount> inline bool BitWriter::Write(const BitValue<Underlying, BitsCount>& value)
{
    return Write(value.Value(), BitsCount);
}

template <typename T> bool BitWriter::Write(gsl::span<T> list)
{
    for (auto value : list)
    {
        if (!Write(value))
        {
            return false;
        }
    }

    return true;
}

template <typename T, size_t N> bool BitWriter::Write(const std::array<T, N>& list)
{
    for (auto value : list)
    {
        if (!Write(value))
        {
            return false;
        }
    }

    return true;
}

template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type> inline bool BitWriter::Write(T value)
{
    return Write(num(value));
}

template <typename T, typename std::enable_if<std::is_fundamental<T>::value, int>::type> inline bool BitWriter::Write(T value)
{
    return Write(static_cast<std::make_unsigned_t<T>>(value));
}

template <std::size_t Size> bool BitWriter::Write(const std::bitset<Size>& value)
{
    for (auto i = 0U; i < Size; i++)
    {
        if (!Write(value[i]))
        {
            return false;
        }
    }

    return true;
}

inline void BitWriter::Initialize(gsl::span<std::uint8_t> view)
{
    this->_buffer = std::move(view);
    this->Reset();
}

inline bool BitWriter::Status() const
{
    return this->_isValid;
}

inline std::uint32_t BitWriter::GetBitFraction() const
{
    return this->_bitPosition;
}

inline gsl::span<std::uint8_t> BitWriter::Capture()
{
    if (!this->_isValid)
    {
        return gsl::span<std::uint8_t>();
    }

    return this->_buffer.subspan(0, GetByteDataLength());
}

inline bool BitWriter::Write(std::uint8_t value, std::uint8_t length)
{
    return WriteWord(value, length);
}

inline bool BitWriter::Write(std::uint16_t value, std::uint8_t length)
{
    return WriteWord(value, length);
}

inline bool BitWriter::Write(std::uint32_t value, std::uint8_t length)
{
    return WriteDoubleWord(value, length);
}

inline bool BitWriter::Write(std::uint64_t value, std::uint8_t length)
{
    return WriteQuadWord(value, length);
}

inline bool BitWriter::Write(bool value)
{
    return WriteWord(value, 1);
}

#endif
