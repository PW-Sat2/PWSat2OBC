from response_frames import ResponseFrame, response_frame
from common import DownlinkApid, GenericErrorResponseFrame, GenericSuccessResponseFrame


@response_frame(0x05)
class PeriodicMessageFrame(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return True

    def decode(self):
        self.message = self.payload()

    def __str__(self):
        return 'Periodic({})'.format(''.join(map(chr, self.message)))

    def __repr__(self):
        return str(self)


@response_frame(DownlinkApid.PeriodicSet)
class PeriodicSetSuccessFrame(GenericSuccessResponseFrame):
    pass


@response_frame(DownlinkApid.PeriodicSet)
class PeriodicSetErrorFrame(GenericErrorResponseFrame):
    pass
