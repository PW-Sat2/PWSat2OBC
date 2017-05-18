import struct
import ctypes

from telecommand.base import Telecommand
from utils import ensure_byte_list


class SetTimeCorrectionConfig(Telecommand):
    def __init__(self, correlation_id, missionTimeWeight, externalTimeWeight):
        self._correlation_id = correlation_id
        self._missionTimeWeight = ensure_byte_list(struct.pack('<H', missionTimeWeight))
        self._externalTimeWeight = ensure_byte_list(struct.pack('<H', externalTimeWeight))

    def apid(self):
        return 0xAE

    def payload(self):
        return [self._correlation_id] + self._missionTimeWeight + self._externalTimeWeight
        