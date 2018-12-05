from devices import UplinkFrame, DownlinkFrame


class Telecommand(object):
    MAX_PAYLOAD_SIZE = UplinkFrame.MAX_PAYLOAD_SIZE

    def __init__(self):
        pass

    def apid(self):
        raise NotImplementedError()

    def payload(self):
        raise NotImplementedError()

    def frame(self):
        return UplinkFrame(apid=self.apid(), content=self.payload())

    def build(self):
        return self.frame().build()

    def __repr__(self):
        return '{}'.format(self.__class__.__name__)


class CorrelatedTelecommand(Telecommand):
    def __init__(self, correlation_id):
        self._correlation_id = correlation_id

    def __repr__(self):
        return "{}, cid={:02d}".format(
            super(CorrelatedTelecommand, self).__repr__(),
            self._correlation_id)
    
    def correlation_id(self):
        return self._correlation_id

class TelecommandResponse(DownlinkFrame):
    def __init__(self, apid, seq, payload):
        super(TelecommandResponse, self).__init__(apid, seq, payload)

    def decode(self):
        raise NotImplementedError()

