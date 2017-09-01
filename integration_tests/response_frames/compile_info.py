from response_frames import ResponseFrame, response_frame


@response_frame(0x08)
class CompileInfoFrame(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return True

    def decode(self):
        self.message = self.payload()

    def __str__(self):
        return 'CompileInfo({})'.format(''.join(map(chr, self.message)))

    def __repr__(self):
        return str(self)