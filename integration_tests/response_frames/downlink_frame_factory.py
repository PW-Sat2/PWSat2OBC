from devices import DownlinkFrame
from marker import BeaconMarker
from exception import NoMatchingFrameType, MultipleMatchingFrameTypes


class DownlinkFrameFactory(object):
    def __init__(self, frame_types):
        self._types = frame_types

    def matches(self, payload):
        return payload[0] != BeaconMarker()

    def decode(self, payload):
        return self.decode_frame(DownlinkFrame.parse(payload))

    def decode_frame(self, frame):
        matching_apids = filter(lambda t: t.ReceivedAPID == frame.apid(), self._types)
        if not matching_apids:
            raise NoMatchingFrameType(frame)

        matching_payload = filter(lambda t: t.matches(frame.payload()), matching_apids)

        if len(matching_payload) == 0:
            raise NoMatchingFrameType(frame)

        if len(matching_payload) > 1:
            raise MultipleMatchingFrameTypes(frame)

        return matching_payload[0](frame.apid(), frame.seq(), frame.payload())


def response_frame(apid):
    def wrapper(c):
        c.ReceivedAPID = apid
        return c

    return wrapper


class ResponseFrame(DownlinkFrame):
    def __init__(self, apid, seq, payload):
        super(ResponseFrame, self).__init__(apid, seq, payload)

        self.decode()

    def decode(self):
        pass

    def __str__(self):
        return "[{}] {}".format(type(self), DownlinkFrame.__str__(self))