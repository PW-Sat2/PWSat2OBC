from response_frames import response_frame, ResponseFrame


@response_frame(0x21)
class MemoryContent(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return True

    def decode(self):
        self.correlation_id = self.payload()[0]
        self.content = self.payload()[1:]

    def __str__(self):
        return 'Memory content (Correlation {}, Seq: {})'.format(self.correlation_id, self.seq())

    def __repr__(self):
        return 'Memory: CID={} Seq={}'.format(self.correlation_id, self.seq())
