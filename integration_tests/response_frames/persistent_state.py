from response_frames import ResponseFrame, response_frame

@response_frame(0x06)
class PersistentStateFrame(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return True

    def decode(self):
        pass
