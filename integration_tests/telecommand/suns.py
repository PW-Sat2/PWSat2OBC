from telecommand.base import Telecommand


class GetSunSDataSets(Telecommand):
    def __init__(self, correlation_id, gain, itime):
        self._correlation_id = correlation_id
        self._gain = gain
        self._itime = itime

    def apid(self):
        return 0xD0

    def payload(self):
        return [self._correlation_id, self._gain, self._itime]
