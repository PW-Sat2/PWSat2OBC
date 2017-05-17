from devices import DownlinkFrame


class NoMatchingFrameType(Exception):
    def __init__(self, frame):
        super(NoMatchingFrameType, self).__init__()
        self._frame = frame

    def __str__(self):
        return 'No matching frame type for frame {}'.format(self._frame)


class MultipleMatchingFrameTypes(Exception):
    def __init__(self, frame):
        super(MultipleMatchingFrameTypes, self).__init__()
        self._frame = frame

    def __str__(self):
        return 'Multiple matching frame types for frame {}'.format(self._frame)


class FrameDecoder:
    def __init__(self):
        self._types = []

    def add_frame_types(self, types):
        self._types += types

    def decode(self, frame):
        matching_apid = filter(lambda t: t.ReceivedAPID == frame.apid(), self._types)

        if not matching_apid:
            raise NoMatchingFrameType()

        matching_payload = filter(lambda t: t.matches(frame.payload()), matching_apid)

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
