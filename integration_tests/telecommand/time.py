import struct
from datetime import timedelta

from telecommand.base import CorrelatedTelecommand
from utils import ensure_byte_list


class SetTimeCorrectionConfig(CorrelatedTelecommand):
    def __init__(self, correlation_id, missionTimeWeight, externalTimeWeight):
        super(SetTimeCorrectionConfig, self).__init__(correlation_id)
        self._missionTimeWeight = missionTimeWeight
        self._externalTimeWeight = externalTimeWeight

    def apid(self):
        return 0x90

    def payload(self):
        return struct.pack('<BHH', self._correlation_id, self._missionTimeWeight, self._externalTimeWeight)

class SetTime(CorrelatedTelecommand):
    def __init__(self, correlation_id, new_time):
        super(SetTime, self).__init__(correlation_id)
        # type: (int, timedelta) -> None
        self._newTime = new_time

    def apid(self):
        return 0x91

    def payload(self):
        return struct.pack('<BI', self._correlation_id, self._newTime.seconds)
