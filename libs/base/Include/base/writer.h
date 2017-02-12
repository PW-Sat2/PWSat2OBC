#ifndef LIBS_BASE_WRITER_H
#define LIBS_BASE_WRITER_H

#include <stdbool.h>
#include <stddef.h>
#include <cstdint>
#include "gsl/span"
#include "system.h"

/**
 * @addtogroup utilities
 * @{
 */

/**
 * @brief Writer object definition.
 *
 * This object is supposed to provide means of writing various entries in a way
 * that is independent of both current memory layout and the actual
 * buffer location in memory.
 */
class Writer final
{
  public:
    /**
     * @brief Default ctor
     */
    Writer();

    /**
     * @brief Initializes generic buffer reader.
     *
     * @param[in] view Window into memory buffer to which the data is written.
     */
    Writer(gsl::span<std::uint8_t> view);

    /**
     * @brief Initializes generic buffer writter.
     *
     * @param[in] view Window into memory buffer to which the data is written.
     */
    void Initialize(gsl::span<std::uint8_t> view);

    /**
     * @brief Returns current writer status.
     * @retval true Buffer is valid and there is still some space left in it.
     * @retval false An attempt to write data beyond the buffer end has been made.
     */
    inline bool Status() const;

    /**
     * @brief Returns the number of not bytes already written to the buffer.
     * @return Number of bytes already written to the buffer.
     */
    inline std::uint16_t GetDataLength() const;

    /**
     * @brief Returns the number of not yet used bytes in the buffer.
     * @retval true Buffer is valid and there is still some unread data left in it.
     * @retval false An attempt to write data beyond the buffer end has been made.
     */
    std::int32_t RemainingSize() const;

    /**
     * @brief Appends single byte to the buffer and moves the current position to the next byte.
     * @param[in] byte Byte that should be added to writer output.
     * @return Operation status.
     */
    bool WriteByte(std::uint8_t byte);

    /**
     * @brief Writes single 16 bit word with little-endian memory orientation to the buffer
     * and advances the current buffer position to the next unused byte.
     * @param[in] word Word that should be added to writer output using little endian byte ordering.
     * @return Operation status.
     */
    bool WriteWordLE(std::uint16_t word);

    /**
     * @brief Writes single 32 bit word with little-endian memory orientation to the buffer
     * and advances the current buffer position to the next unused byte.
     * @param[in] dword Doubleword that should be added to writer output using little endian byte ordering.
     * @return Operation status.
     */
    bool WriteDoubleWordLE(std::uint32_t dword);

    /**
     * @brief Writes single 16 bit signed value with little-endian memory orientation to the buffer
     * and advances the current buffer position to the next unused byte.
     * Value is written in 2's complement notation.
     * @param[in] word Word that should be added to writer output using little endian byte ordering.
     * @return Operation status.
     */
    bool WriteSignedWordLE(std::int16_t word);

    /**
     * @brief Writes single 64 bit word with little-endian memory orientation to the buffer
     * and advances the current buffer position to the next unused byte.
     * @param[in] dword Quadword that should be added to writer output using little endian byte ordering.
     * @return Operation status.
     */
    bool WriteQuadWordLE(std::uint64_t dword);

    /**
     * @brief Writes single 32 bit signed value with little-endian memory orientation to the buffer
     * and advances the current buffer position to the next unused byte.
     * Value is written in 2's complement notation.
     * @param[in] dword Doubleword that should be added to writer output using little endian byte ordering.
     * @return Operation status.
     */
    bool WriteSignedDoubleWordLE(std::int32_t dword);

    /**
     * @brief Writes the requested memory block to the buffer.
     *
     * @param[in] buffer Span whose contents should be appended to writer output.
     * @return Operation status.
     */
    bool WriteArray(gsl::span<const std::uint8_t> buffer);

    /**
     * @brief Writes lower bytes of specified number in big-endian order
     * @param[in] number Number to write
     * @param[in] bytesCount Number of lower bytes to write
     * @return Operation status
     */
    bool WriteLowerBytesBE(std::uint32_t number, std::uint8_t bytesCount);

    gsl::span<std::uint8_t> Reserve(std::size_t count);

    /**
     * @brief Returns view for used part of buffer
     * @return Span covering used part of buffer
     */
    gsl::span<std::uint8_t> Capture();

  private:
    /**
     * @brief Updates internal writer status
     * @param requestedSize Number of bytes that will be written to buffer
     * @retval true Write operation can be performed
     * @retval false Write operation cannot be performed
     */
    bool UpdateState(std::uint8_t requestedSize);

    /**
     * @brief Resets reader to the initial state.
     */
    inline void Reset();

    /**
     * @brief Pointer to the buffer in memory.
     */
    gsl::span<std::uint8_t> _buffer;

    /**
     * @brief Current buffer location.
     *
     * This value points to the first not yet processed byte.
     */
    std::uint16_t _position;

    /**
     * @brief Current buffer status.
     *
     *  - True -> Data is valid.
     *  - False -> Buffer overflow detected.
     */
    bool _isValid;
};

inline bool Writer::Status() const
{
    return this->_isValid;
}

inline void Writer::Reset()
{
    this->_position = 0;
    this->_isValid = this->_buffer.length() > 0;
}

inline uint16_t Writer::GetDataLength() const
{
    return this->_position;
}

/** @}*/

#endif
