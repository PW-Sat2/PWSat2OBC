from response_frames import ResponseFrame, response_frame


@response_frame(0x07)
class SailExperimentFrame(ResponseFrame):
    @classmethod
    def matches(cls, _):
        return True

    def decode(self):
        self.response = self.payload()
