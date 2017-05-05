import struct

from telecommand.base import Telecommand
from utils import ensure_byte_list


class EnterIdleState(Telecommand):
    def __init__(self, correlation_id, duration):
        self._correlation_id = correlation_id
        self._duration = duration

    def apid(self):
        return 0xAD

    def payload(self):
        return [self._correlation_id, self._duration]
