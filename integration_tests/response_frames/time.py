from response_frames import response_frame, ResponseFrame
from response_frames.common import GenericSuccessResponseFrame, GenericErrorResponseFrame, DownlinkApid


@response_frame(DownlinkApid.TimeCorrection)
class TimeCorrectionSuccessFrame(GenericSuccessResponseFrame):
    pass


@response_frame(DownlinkApid.TimeCorrection)
class TimeCorrectionErrorFrame(GenericErrorResponseFrame):
    pass


@response_frame(DownlinkApid.TimeSet)
class TimeSetSuccessFrame(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return len(payload) >= 2 and payload[1] >= 0

    def decode(self):
        self.correlation_id = self.payload()[0]
        self.response = self.payload()[2:]


@response_frame(DownlinkApid.TimeSet)
class TimeSetErrorFrame(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return len(payload) >= 2 and payload[1] < 0

    def decode(self):
        self.correlation_id = self.payload()[0]
        self.error_code = self.payload()[1]
        self.response = self.payload()[2:]