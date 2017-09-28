import struct

from telecommand.base import Telecommand


class ReadMemory(Telecommand):
    def __init__(self, correlation_id, offset, size):
        self.size = size
        self.offset = offset
        self.correlation_id = correlation_id

    def apid(self):
        return 0x29

    def payload(self):
        return struct.pack('<BII', self.correlation_id, self.offset, self.size)
