import struct

from telecommand import Telecommand


class EraseBootTableEntry(Telecommand):
    def apid(self):
        return 0xB0

    def payload(self):
        mask = 0
        for e in self._entries:
            mask |= 1 << (e - 1)

        return [mask]

    def __init__(self, entries):
        self._entries = entries


class WriteProgramPart(Telecommand):
    MAX_PART_SIZE = Telecommand.MAX_PAYLOAD_SIZE - 3

    def apid(self):
        return 0xB1

    def payload(self):
        mask = 0
        for e in self._entries:
            mask |= 1 << (e - 1)

        return list(struct.pack('<BI', mask, self._offset)) + list(self._content)

    def __init__(self, entries, offset, content):
        self._offset = offset
        self._content = content
        self._entries = entries


class FinalizeProgramEntry(Telecommand):
    def apid(self):
        return 0xB2

    def payload(self):
        mask = 0
        for e in self._entries:
            mask |= 1 << (e - 1)

        return list(struct.pack('<BIH', mask, self._length, self._expected_crc)) + list(self._name)

    def __init__(self, entries, length, expected_crc, name):
        self._entries = entries
        self._length = length
        self._expected_crc = expected_crc
        self._name = name
