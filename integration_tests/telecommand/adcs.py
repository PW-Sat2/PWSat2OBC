import struct
from telecommand import CorrelatedTelecommand
from devices.adcs import AdcsMode


class SetBuiltinDetumblingBlockMaskTelecommand(CorrelatedTelecommand):
    def __init__(self, correlation_id, mask):
        CorrelatedTelecommand.__init__(self, correlation_id)
        self._mask = mask

    def apid(self):
        return 0x24

    def payload(self):
        return [self._correlation_id, self._mask]


class SetAdcsModeTelecommand(CorrelatedTelecommand):
    def __init__(self, correlation_id, mode):
        CorrelatedTelecommand.__init__(self, correlation_id)
        self._mode = mode

    def apid(self):
        return 0x25

    def payload(self):
        return [self._correlation_id, struct.pack('<b', self._mode.value) if type(self._mode) == AdcsMode else self._mode]

    def __repr__(self):
        return "{}, mode={}".format(
            super(SetAdcsModeTelecommand, self).__repr__(),
            self._mode)