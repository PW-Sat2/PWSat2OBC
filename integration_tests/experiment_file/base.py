from parsec import *

from parsing import *


def pid(n):
    @Parser
    def match_pid(text, index=0):
        if index < len(text) and text[index] == chr(n):
            return Value.success(index + 1, text[index])
        else:
            return Value.failure(index, 'PID {:02X}'.format(n))

    return match_pid


def bytes_block(n):
    return count(byte, n).parsecmap(lambda x: ''.join(x)).parsecmap(hexlify)

Synchronization = pid(0x47).result('Synchronization')

Timestamp = pid(1) >> packed('<Q').parsecmap(lambda x: timedelta(milliseconds=x))
Timestamp >>= label_as('time')

Padding = many1(pid(0xFF)).parsecmap(lambda x: len(x))
Padding >>= label_as('Padding')
