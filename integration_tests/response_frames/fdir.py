from response_frames import response_frame, ResponseFrame


@response_frame(0x03)
class ErrorCountersFrame(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return True

    def decode(self):
        self.counters = {}

        raw = self.payload()

        for i in xrange(0, len(raw) / 4):
            part = raw[4 * i: 4 * i + 4]

            self.counters[i] = {
                'device': i,
                'current': part[0],
                'limit': part[1],
                'increment': part[2],
                'decrement': part[3]
            }

    def __str__(self):
        return 'Error Counters'

