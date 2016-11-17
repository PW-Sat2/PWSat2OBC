#ifndef LIBS_BASE_READER_H
#define LIBS_BASE_READER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "system.h"

EXTERNC_BEGIN

/**
 * @defgroup utilities General Purpose Utilities
 * @{
 */

/**
 * @brief Reader object definition.
 *
 * This object is supposed to provide means of reading various entries in a way
 * that is independent of both current memory layout and the actual
 * buffer location in memory.
 */
typedef struct
{
    /**
     * @brief Pointer to the buffer in memory.
     */
    const uint8_t* buffer;

    /**
     * @brief Buffer length in bytes.
     */
    uint16_t length;

    /**
     * @brief Current buffer location.
     *
     * This value points to the first not yet processed byte.
     */
    uint16_t position;

    /**
     * @brief Current buffer status.
     *
     *  - True -> Data is valid.
     *  - False -> Buffer overflow detected.
     */
    bool isValid;
} Reader;

/**
 * @brief Initializes generic buffer reader.
 *
 * @param[in] reader Pointer to reader object that should be initialized.
 * @param[in] buffer Buffer that should be read from.
 * @param[in] length The size in bytes of the data stored in the buffer.
 */
void ReaderInitialize(Reader* reader, const uint8_t* buffer, uint16_t length);

/**
 * @brief Returns current reader status.
 * @param[in] reader Pointer to the queried reader object.
 * @retval true Buffer is valid and there is still some unread data left in it.
 * @retval false An attempt to read beyond the buffer end has been made.
 */
static bool ReaderStatus(const Reader* reader);

/**
 * @brief Returns the number of not yet read bytes from the buffer.
 * @param[in] reader Pointer to the queried reader object.
 * @retval true Buffer is valid and there is still some unread data left in it.
 * @retval false An attempt to read beyond the buffer end has been made.
 */
int32_t ReaderRemainingSize(const Reader* reader);

/**
 * @brief Read single byte from the buffer and move the current position to the next byte.
 * @param[in] reader Pointer to the queried reader object.
 * @return Read byte.
 */
uint8_t ReaderReadByte(Reader* reader);

/**
 * @brief Read single 16 bit word with little-endian memory orientation from the buffer
 * and advance the current buffer position to the next unread byte.
 * @param[in] reader Pointer to the queried reader object.
 * @return Read word.
 */
uint16_t ReaderReadWordLE(Reader* reader);

/**
 * @brief Read single 16 bit word with big-endian memory orientation from the buffer
 * and advance the current buffer position to the next unread byte.
 * @param[in] reader Pointer to the queried reader object.
 * @return Read word.
 */
uint16_t ReaderReadWordBE(Reader* reader);

/**
 * @brief Read single 32 bit word with little-endian memory orientation from the buffer
 * and advance the current buffer position to the next unread byte.
 * @param[in] reader Pointer to the queried reader object.
 * @return Read double word.
 */
uint32_t ReaderReadDoubleWordLE(Reader* reader);

/**
 * @brief Read single 64 bit word with little-endian memory orientation from the buffer
 * and advance the current buffer position to the next unread byte.
 * @param[in] reader Pointer to the queried reader object.
 * @return Read double word.
 */
uint64_t ReaderReadQuadWordLE(Reader* reader);

/**
 * @brief Read the requested number of bytes from the buffer.
 *
 * This method does not perform any operation/transformation on the data it only ensures that
 * there are requested number of bytes available in the buffer and advances the current
 * buffer position to the first byte beyond the requested block.
 * @param[in] reader Pointer to the queried reader object.
 * @param[in] length Size in bytes of the requested data block.
 * @return Pointer to the first byte of the requested memory block.
 */
const uint8_t* ReaderReadArray(Reader* reader, uint16_t length);

/**
 * @brief Resets reader to the initial state.
 * @param[in] reader Pointer to the reader that should be reset.
 */
static void ReaderReset(Reader* reader);

static inline bool ReaderStatus(const Reader* reader)
{
    return reader->isValid;
}

static inline void ReaderReset(Reader* reader)
{
    reader->position = 0;
    reader->isValid = reader->buffer != NULL && reader->length > 0;
}

/** @}*/
EXTERNC_END

#endif
