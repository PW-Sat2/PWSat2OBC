from common import GenericSuccessResponseFrame, DownlinkApid
from response_frames import response_frame, ResponseFrame


@response_frame(DownlinkApid.LittleOryxReboot)
class RebootSuccessFrame(GenericSuccessResponseFrame):
    pass


@response_frame(DownlinkApid.LittleOryxDelayReboot)
class DelayRebootSuccessFrame(GenericSuccessResponseFrame):
    pass


@response_frame(DownlinkApid.LittleOryxEcho)
class EchoFrame(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return True

    def decode(self):
        self.message = self.payload()

    def __str__(self):
        return 'Echo({})'.format(''.join(map(chr, self.message)))

    def __repr__(self):
        return str(self)
