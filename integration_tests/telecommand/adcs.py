import struct
from telecommand import Telecommand
from devices.adcs import AdcsMode


class SetBuiltinDetumblingBlockMaskTelecommand(Telecommand):
    def __init__(self, corelation_id, mask):
        Telecommand.__init__(self)
        self._mask = mask
        self._corelation_id = corelation_id

    def apid(self):
        return 0x24

    def payload(self):
        return [self._corelation_id, self._mask]

class SetAdcsModeTelecommand(Telecommand):
    def __init__(self, corelation_id, mode):
        Telecommand.__init__(self)
        self._mode = mode
        self._corelation_id = corelation_id

    def apid(self):
        return 0x25

    def payload(self):
        return [self._corelation_id, struct.pack('<b', self._mode.value) if type(self._mode) == AdcsMode else self._mode]
