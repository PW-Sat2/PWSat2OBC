from frame_decoder import FrameDecoder
from beacon_factory import BeaconFrameFactory
from downlink_frame_factory import DownlinkFrameFactory, ResponseFrame, response_frame
from exception import MultipleMatchingFrameTypes, NoMatchingFrameType
import inspect

import pong
import operation
import program_upload
import fdir
import period_message
import persistent_state

frame_types = []
frame_types += map(lambda t: t[1], inspect.getmembers(pong, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(operation, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(program_upload, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(fdir, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(period_message, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(persistent_state, predicate=inspect.isclass))
frame_types = filter(lambda t: issubclass(t, ResponseFrame) and t != ResponseFrame, frame_types)

frame_factories = [BeaconFrameFactory(), DownlinkFrameFactory(frame_types)]
    
__all__ = [
    'ResponseFrame',
    'response_frame',
    'FrameDecoder',
    'BeaconFrame',
    'PersistentStateFrame',
    'MultipleMatchingFrameTypes',
    'NoMatchingFrameType',
    'frame_factories'
]
