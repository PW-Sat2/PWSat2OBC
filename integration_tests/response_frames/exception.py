
class NoMatchingFrameType(Exception):
    def __init__(self, frame):
        super(NoMatchingFrameType, self).__init__()
        self._frame = frame

    def __str__(self):
        return 'No matching frame type for frame {}'.format(self._frame)

class MultipleMatchingFrameTypes(Exception):
    def __init__(self, frame):
        super(MultipleMatchingFrameTypes, self).__init__()
        self._frame = frame

    def __str__(self):
        return 'Multiple matching frame types for frame {}'.format(self._frame)