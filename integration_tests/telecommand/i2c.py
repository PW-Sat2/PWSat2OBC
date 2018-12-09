import struct

from telecommand.base import CorrelatedTelecommand
from utils import ensure_byte_list


class RawI2C(CorrelatedTelecommand):
    def __init__(self, correlation_id, busSelect, address, delay, data):
        super(RawI2C, self).__init__(correlation_id)
        self._busSelect = busSelect
        self._address = address
        self._delay = delay
        self._data = data

    def apid(self):
        return 0x80

    def payload(self):
        return ensure_byte_list(struct.pack('<BBBH' + 'B' * len(self._data), self._correlation_id, self._busSelect, self._address, self._delay, *self._data))
