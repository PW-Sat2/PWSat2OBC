from response_frames import ResponseFrame, response_frame
from common import DownlinkApid


@response_frame(DownlinkApid.SunS)
class SunSDataSetsFrame(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return len(payload) == 94

    def decode(self):
        self.correlation_id = self.payload()[0]
        self.response = self.payload()[1:]
