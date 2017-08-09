from response_frames import ResponseFrame, response_frame


@response_frame(0x02)
class OperationSuccessFrame(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return len(payload) >= 2 and payload[1] == 0

    def decode(self):
        self.correlation_id = self.payload()[0]
        self.response = self.payload()[1:]


@response_frame(0x02)
class OperationErrorFrame(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return len(payload) >= 2 and payload[1] != 0

    def decode(self):
        self.correlation_id = self.payload()[0]
        self.error_code = self.payload()[1]
        self.response = self.payload()[2:]
