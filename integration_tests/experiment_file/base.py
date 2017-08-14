import struct
from binascii import hexlify
from datetime import timedelta

from parsec import *


def pid(n):
    @Parser
    def match_pid(text, index=0):
        if index < len(text) and text[index] == chr(n):
            return Value.success(index + 1, text[index])
        else:
            return Value.failure(index, 'PID {:02X}'.format(n))

    return match_pid


@Parser
def byte(text, index=0):
    if index < len(text):
        return Value.success(index + 1, text[index])
    else:
        return Value.failure(index, 'a byte')


def bytes_block(n):
    return count(byte, n).parsecmap(lambda x: ''.join(x)).parsecmap(hexlify)


def packed(fmt):
    size = struct.calcsize(fmt)

    @Parser
    def unpack(text, index=0):
        if index + size < len(text):
            part = text[index: index + size]
            unpacked = struct.unpack(fmt, part)
            return Value.success(index + size, unpacked[0])
        else:
            return Value.failure(index, 'Packed value: {}'.format(fmt))

    return unpack


def label_as(name):
    def as_dict(value):
        return Parser(lambda _, index: Value.success(index, {name: value}))

    return as_dict


def to_dict(value):
    return Parser(lambda _, index: Value.success(index, dict(value)))


def field(name, content_parser):
    return content_parser.parsecmap(lambda v: (name, v))


boolean = packed('<?')

uint16 = packed('<H')
int16 = packed('<h')
int32 = packed('<l')

Synchronization = pid(0x47).result('Synchronization')

Timestamp = pid(1) >> packed('<Q').parsecmap(lambda x: timedelta(milliseconds=x))
Timestamp >>= label_as('time')

Padding = many1(pid(0xFF)).parsecmap(lambda x: len(x))
Padding >>= label_as('Padding')
