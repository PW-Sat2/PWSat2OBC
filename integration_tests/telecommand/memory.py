import struct

from telecommand.base import Telecommand, CorrelatedTelecommand


class ReadMemory(CorrelatedTelecommand):
    def __init__(self, correlation_id, offset, size):
        super(ReadMemory, self).__init__(correlation_id)
        self.size = size
        self.offset = offset

    def apid(self):
        return 0x29

    def payload(self):
        return struct.pack('<BII', self._correlation_id, self.offset, self.size)
