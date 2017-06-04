from response_frames import ResponseFrame, response_frame

@response_frame(0x03)
class BeaconFrame(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        # TODO update this condition to match the current beacon content
        return len(payload) >= 128

    def decode(self):
        pass