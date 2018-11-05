import struct

from telecommand.base import Telecommand, CorrelatedTelecommand
from utils import ensure_byte_list


class SetPeriodicMessageTelecommand(CorrelatedTelecommand):
    def __init__(self, correlation_id, interval_minutes, repeat_count, message):
        super(SetPeriodicMessageTelecommand, self).__init__(correlation_id)
        self._message = message
        self._repeat_count = repeat_count
        self._interval_minutes = interval_minutes

    def apid(self):
        return 0x05

    def payload(self):
        return [self._correlation_id, self._interval_minutes, self._repeat_count] + ensure_byte_list(self._message)


class SendPeriodicMessageTelecommand(Telecommand):
    def __init__(self, count):
        self.count = count

    def apid(self):
        return 0x23

    def payload(self):
        return struct.pack('<B', self.count)
