from devices import UplinkFrame, DownlinkFrame


class Telecommand:
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


class TelecommandResponse(DownlinkFrame):
    def __init__(self, apid, seq, payload):
        super(TelecommandResponse, self).__init__(apid, seq, payload)

    def decode(self):
        raise NotImplemented()

