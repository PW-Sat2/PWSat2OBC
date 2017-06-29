from exception import NoMatchingFrameType

class FrameDecoder:
    def __init__(self, types):
        self._types = types

    def decode(self, frame):
        matching_type = filter(lambda t: t.matches(frame), self._types)

        if not matching_type:
            raise NoMatchingFrameType(frame)

        return matching_type[0].decode(frame)

