from frame_decoder import ResponseFrame, response_frame, FrameDecoder, MultipleMatchingFrameTypes, NoMatchingFrameType
import inspect

import pong
import operation
import program_upload

frame_types = []
frame_types += map(lambda t: t[1], inspect.getmembers(pong, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(operation, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(program_upload, predicate=inspect.isclass))

frame_types = filter(lambda t: issubclass(t, ResponseFrame) and t != ResponseFrame, frame_types)

__all__ = [
    'ResponseFrame',
    'response_frame',
    'FrameDecoder',
    'MultipleMatchingFrameTypes',
    'NoMatchingFrameType',
    'frame_types'
]