import struct

from .base import Telecommand


class PerformDetumblingExperiment(Telecommand):
    def apid(self):
        return 0x0D

    def payload(self):
        return struct.pack('<L', self._duration.total_seconds())

    def __init__(self, duration):
        self._duration = duration


class AbortExperiment(Telecommand):
    def __init__(self, correlation_id):
        super(AbortExperiment, self).__init__()
        self.correlation_id = correlation_id

    def apid(self):
        return 0x0E

    def payload(self):
        return [self.correlation_id]
