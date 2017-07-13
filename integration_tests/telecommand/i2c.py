import struct

from telecommand.base import Telecommand
from utils import ensure_byte_list


class RawI2C(Telecommand):
    def __init__(self, correlation_id, busSelect, address, delay, data):
        self._correlation_id = ensure_byte_list(struct.pack('B', correlation_id))
        self._busSelect = ensure_byte_list(struct.pack('B', busSelect))
        self._address = ensure_byte_list(struct.pack('B', address))
        self._delay = ensure_byte_list(struct.pack('<H', delay))
        self._data = ensure_byte_list(struct.pack('B' * len(data), *data))

    def apid(self):
        return 0x80

    def payload(self):
        return self._correlation_id + self._busSelect + self._address + self._delay + self._data
