#include "writer.h"
#include <string.h>

static bool WriterUpdateState(Writer* writer, uint8_t requestedSize)
{
    if (writer->status)
    {
        writer->status = (writer->position + requestedSize) <= writer->length;
    }

    return writer->status;
}

void WriterInitialize(Writer* writer, uint8_t* buffer, uint16_t length)
{
    writer->buffer = buffer;
    writer->length = length;
    WriterReset(writer);
}

int32_t WriterRemainingSize(const Writer* writer)
{
    int32_t result = writer->length;
    return result - writer->position;
}

bool WriterWriteByte(Writer* writer, uint8_t byte)
{
    if (!WriterUpdateState(writer, 1))
    {
        return false;
    }
    else
    {
        writer->buffer[writer->position++] = byte;
        return true;
    }
}

bool WriterWriteWordLE(Writer* writer, uint16_t word)
{
    if (!WriterUpdateState(writer, 2))
    {
        return false;
    }
    else
    {
        writer->buffer[writer->position] = word & 0xff;
        writer->buffer[writer->position + 1] = (word >> 8) & 0xff;
        writer->position += 2;
        return true;
    }
}

bool WriterWriteSignedWordLE(Writer* writer, int16_t word)
{
    return WriterWriteWordLE(writer, (uint16_t)(word));
}

bool WriterWriteDoubleWordLE(Writer* writer, uint32_t word)
{
    if (!WriterUpdateState(writer, 4))
    {
        return false;
    }
    else
    {
        writer->buffer[writer->position] = word & 0xff;
        writer->buffer[writer->position + 1] = (word >> 8) & 0xff;
        writer->buffer[writer->position + 2] = (word >> 16) & 0xff;
        writer->buffer[writer->position + 3] = (word >> 24) & 0xff;
        writer->position += 4;
        return true;
    }
}

bool WriterWriteSignedDoubleWordLE(Writer* writer, int32_t dword)
{
    return WriterWriteDoubleWordLE(writer, (uint32_t)(dword));
}

bool WriterWriteQuadWordLE(Writer* writer, uint64_t word)
{
    if (!WriterUpdateState(writer, 8))
    {
        return false;
    }
    else
    {
        writer->buffer[writer->position] = word & 0xff;
        writer->buffer[writer->position + 1] = (word >> 8) & 0xff;
        writer->buffer[writer->position + 2] = (word >> 16) & 0xff;
        writer->buffer[writer->position + 3] = (word >> 24) & 0xff;
        writer->buffer[writer->position + 4] = (word >> 32) & 0xff;
        writer->buffer[writer->position + 5] = (word >> 40) & 0xff;
        writer->buffer[writer->position + 6] = (word >> 48) & 0xff;
        writer->buffer[writer->position + 7] = (word >> 56) & 0xff;
        writer->position += 8;
        return true;
    }
}

bool WriterWriteArray(Writer* writer, const uint8_t* buffer, uint16_t length)
{
    if (!WriterUpdateState(writer, length))
    {
        return false;
    }
    else
    {
        memcpy(writer->buffer + writer->position, buffer, length);
        writer->position += length;
        return true;
    }
}
