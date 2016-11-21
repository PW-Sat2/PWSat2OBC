#include "reader.h"

static bool ReaderUpdateState(Reader* reader, uint8_t requestedSize)
{
    if (reader->status)
    {
        reader->status = (reader->position + requestedSize) <= reader->length;
    }

    return reader->status;
}

void ReaderInitialize(Reader* reader, const uint8_t* buffer, uint16_t length)
{
    reader->buffer = buffer;
    reader->length = length;
    ReaderReset(reader);
}

uint8_t ReaderReadByte(Reader* reader)
{
    if (!ReaderUpdateState(reader, 1))
    {
        return 0;
    }
    else
    {
        return reader->buffer[reader->position++];
    }
}

uint16_t ReaderReadWordLE(Reader* reader)
{
    if (!ReaderUpdateState(reader, 2))
    {
        return 0;
    }
    else
    {
        uint16_t value = reader->buffer[reader->position + 1];
        value <<= 8;
        value += reader->buffer[reader->position];
        reader->position += 2;
        return value;
    }
}

uint32_t ReaderReadDoubleWordLE(Reader* reader)
{
    if (!ReaderUpdateState(reader, 4))
    {
        return 0;
    }
    else
    {
        uint32_t value = reader->buffer[reader->position + 3];
        value <<= 8;
        value += reader->buffer[reader->position + 2];
        value <<= 8;
        value += reader->buffer[reader->position + 1];
        value <<= 8;
        value += reader->buffer[reader->position];
        reader->position += 4;
        return value;
    }
}

uint64_t ReaderReadQuadWordLE(Reader* reader)
{
    if (!ReaderUpdateState(reader, 8))
    {
        return 0;
    }
    else
    {
        uint64_t value = reader->buffer[reader->position + 7];
        value <<= 8;
        value += reader->buffer[reader->position + 6];
        value <<= 8;
        value += reader->buffer[reader->position + 5];
        value <<= 8;
        value += reader->buffer[reader->position + 4];
        value <<= 8;
        value += reader->buffer[reader->position + 3];
        value <<= 8;
        value += reader->buffer[reader->position + 2];
        value <<= 8;
        value += reader->buffer[reader->position + 1];
        value <<= 8;
        value += reader->buffer[reader->position];
        reader->position += 8;
        return value;
    }
}

const uint8_t* ReaderReadArray(Reader* reader, uint16_t length)
{
    if (!ReaderUpdateState(reader, length))
    {
        return NULL;
    }
    else
    {
        const uint8_t* value = reader->buffer + reader->position;
        reader->position += length;
        return value;
    }
}

int32_t ReaderRemainingSize(const Reader* reader)
{
    const int32_t size = reader->length;
    return size - reader->position;
}
