import struct
from binascii import hexlify
from datetime import timedelta

from parsec import Parser, Value


def packed(fmt):
    size = struct.calcsize(fmt)

    @Parser
    def unpack(text, index=0):
        if index + size <= len(text):
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

byte = packed('<B')

int16 = packed('<h')
uint16 = packed('<H')

int32 = packed('<l')
uint32 = packed('<L')
