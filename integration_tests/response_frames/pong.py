from response_frames import ResponseFrame, response_frame


@response_frame(0x01)
class PongFrame(ResponseFrame):
    @classmethod
    def matches(cls, _):
        return True
