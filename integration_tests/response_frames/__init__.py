from frame_decoder import FrameDecoder
from beacon_factory import BeaconFrameFactory
from downlink_frame_factory import DownlinkFrameFactory, ResponseFrame, response_frame
from exception import MultipleMatchingFrameTypes, NoMatchingFrameType
from devices.comm_beacon import BeaconFrame
import inspect

from pong import *
from operation import *
from program_upload import *
from fdir import *
from period_message import *
from persistent_state import *
from compile_info import *
from common import *
from suns import *
from disable_overheat_submode import *
from sail_experiment import *
from set_bitrate import *
from adcs import *
from stop_sail_deployment import *
from stop_antenna_deployment import *
from memory import *
from file_system import *
from comm import *
from time import *
from deep_sleep_beacon import * 

frame_types = []
frame_types += map(lambda t: t[1], inspect.getmembers(pong, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(operation, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(common, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(program_upload, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(fdir, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(period_message, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(persistent_state, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(compile_info, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(disable_overheat_submode, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(suns, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(sail_experiment, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(set_bitrate, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(adcs, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(stop_sail_deployment, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(memory, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(file_system, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(comm, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(time, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(stop_antenna_deployment, predicate=inspect.isclass))
frame_types += map(lambda t: t[1], inspect.getmembers(deep_sleep_beacon, predicate=inspect.isclass))
frame_types = filter(lambda t: issubclass(t, ResponseFrame) and t != ResponseFrame, frame_types)
frame_types = reduce(lambda t, x: t + [x] if x not in t else t, frame_types, [])

frame_factories = [BeaconFrameFactory(), DownlinkFrameFactory(frame_types)]
    
__all__ = [
    'ResponseFrame',
    'response_frame',
    'FrameDecoder',
    'BeaconFrame',
    'MultipleMatchingFrameTypes',
    'NoMatchingFrameType',
    'frame_factories'
] + map(lambda t: t.__name__, frame_types)
