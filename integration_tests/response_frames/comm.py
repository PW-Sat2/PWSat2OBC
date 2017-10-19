from response_frames import response_frame, ResponseFrame


@response_frame(0x22)
class OperationSuccessFrame(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return True

