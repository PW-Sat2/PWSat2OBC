from telecommand.base import Telecommand, CorrelatedTelecommand
import struct

class EnterIdleState(CorrelatedTelecommand):
    def __init__(self, correlation_id, duration):
        super(EnterIdleState, self).__init__(correlation_id)
        self._duration = duration

    def apid(self):
        return 0xAD

    def payload(self):
        return [self._correlation_id, self._duration]


class SendBeacon(Telecommand):
    def __init__(self):
        pass

    def apid(self):
        return 0xAE

    def payload(self):
        return []


class ResetTransmitterTelecommand(Telecommand):
    def __init__(self):
        pass

    def apid(self):
        return 0xAF

    def payload(self):
        return []


class SetBitrate(CorrelatedTelecommand):
    def __init__(self, correlation_id, bitrate):
        super(SetBitrate, self).__init__(correlation_id)
        self._bitrate = bitrate
    
    def apid(self):
        return 0x18
    
    def payload(self):
        return struct.pack('<BB', self._correlation_id, int(self._bitrate))

    def __repr__(self):
        return "{}, bitrate={}".format(
            super(SetBitrate, self).__repr__(),
            self._bitrate)
