from telecommand.base import CorrelatedTelecommand


class GetSunSDataSets(CorrelatedTelecommand):
    def __init__(self, correlation_id, gain, itime):
        super(GetSunSDataSets, self).__init__(correlation_id)
        self._gain = gain
        self._itime = itime

    def apid(self):
        return 0xD0

    def payload(self):
        return [self._correlation_id, self._gain, self._itime]
