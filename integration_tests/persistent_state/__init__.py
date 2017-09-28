from parsec import joint, count

from adcs import *
from antenna import *
from boot import *
from error_counters import *
from message import *
from sail import *
from time import *

PersistentStateParser = joint(
    AntennaConfiguration,
    MissionTime, 
    TimeCorrection, 
    BootState, 
    SailState,
    ErrorCounters,
    AdcsConfiguration,
    Message
    )

__all__ = [
    'PersistentStateParser'
]
