#ifndef LIBS_BASE_WRITER_H
#define LIBS_BASE_WRITER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Writer object definition.
 *
 * This object is supposed to provide mens of writing various entries in a way
 * that is independent of both current memory layout and the actual
 * buffer location in memory.
 */
typedef struct
{
    /**
     * @brief Pointer to the buffer in memory.
     */
    uint8_t* buffer;

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
     * True -> Data is valid.
     * False -> Buffer overflow detected.
     */
    bool status;
} Writer;

/**
 * @brief Initializes generic buffer reader.
 *
 * @param[in] writer Pointer to writer object that should be initialized.
 * @param[in] buffer Buffer that should be read from.
 * @param[in] length The size in bytes of the data stored in the buffer.
 */
void WriterInitialize(Writer* writer, uint8_t* buffer, uint16_t length);

/**
 * @brief Returns current writer status.
 * @param[in] writer Pointer to the queried writer object.
 * @retval true Buffer is valid and there is still some space left in it.
 * @retval false An attempt to write data beyond the buffer end has been made.
 */
static bool WriterStatus(const Writer* writer);

/**
 * @brief Returns the number of not bytes already written to the buffer.
 * @param[in] writer Pointer to the queried writer object.
 * return Number of not bytes already written to the buffer.
 */
static uint16_t WriterGetDataLength(const Writer* writer);

/**
 * @brief Returns the number of not yet used bytes in the buffer.
 * @param[in] writer Pointer to the queried writer object.
 * @retval true Buffer is valid and there is still some unread data left in it.
 * @retval false An attempt to write data beyond the buffer end has been made.
 */
int32_t WriterRemainingSize(const Writer* writer);

/**
 * @brief Appends single byte to the buffer and moves the current position to the next byte.
 * @param[in] writer Pointer to the writer object.
 * @return Operation status.
 */
bool WriterWriteByte(Writer* writer, uint8_t byte);

/**
 * @brief Writes single 16 bit word with little-endian memory orientation to the buffer
 * and advances the current buffer position to the next unused byte.
 * @param[in] writer Pointer to the writer object.
 * @return Operation status.
 */
bool WriterWriteWordLE(Writer* writer, uint16_t word);

/**
 * @brief Writes single 32 bit word with little-endian memory orientation to the buffer
 * and advances the current buffer position to the next unused byte.
 * @param[in] writer Pointer to the queried reader object.
 * @return Operation status.
 */
bool WriterWriteDoubleWordLE(Writer* writer, uint32_t dword);

/**
 * @brief Writes the requested memory block to the buffer.
 *
 * @param[in] writer Pointer to the writer object.
 * @return Operation status.
 */
bool WriterWriteArray(Writer* writer, const uint8_t* buffer, uint16_t length);

/**
 * @brief Resets reader to the initial state.
 * @param[in] writer Pointer to the writer that should be reset.
 */
static void WriterReset(Writer* writer);

inline bool WriterStatus(const Writer* writer)
{
    return writer->status;
}

inline void WriterReset(Writer* writer)
{
    writer->position = 0;
    writer->status = writer->buffer != NULL && writer->length > 0;
}

inline uint16_t WriterGetDataLength(const Writer* writer)
{
    return writer->position;
}

#ifdef __cplusplus
}
#endif

#endif
