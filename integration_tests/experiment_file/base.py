import struct

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
    return count(byte, n).parsecmap(lambda x: ''.join(x))


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


Synchronization = pid(0x47).result('Synchronization')

Timestamp = pid(1) >> packed('<Q')
Timestamp >>= label_as('time')

Padding = many1(pid(0xFF)).parsecmap(lambda x: len(x))
Padding >>= label_as('Padding')
