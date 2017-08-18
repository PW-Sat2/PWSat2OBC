from parsec import joint, count

from antenna import *
from boot import *
from error_counters import *
from message import *
from sail import *
from time import *

PersistentStateParser = joint(
    count(AntennaConfiguration, 1),
    count(MissionTime, 1),
    count(TimeCorrection, 1),
    count(BootState, 1),
    count(SailState, 1),
    count(ErrorCounters, 1),
    count(Message, 1)
    )

__all__ = [
    'PersistentStateParser'
]
