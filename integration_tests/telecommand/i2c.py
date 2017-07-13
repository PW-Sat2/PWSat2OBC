import struct

from telecommand.base import Telecommand
from utils import ensure_byte_list


class RawI2C(Telecommand):
    def __init__(self, correlation_id, address, data):
        self._correlation_id = correlation_id
        self._address = address
        self._data = data

    def apid(self):
        return 0x80

    def payload(self):
        data_bytes = ensure_byte_list(struct.pack('B' * len(self._data), *self._data))
        return [self._correlation_id, self._address] + data_bytes
