from parsec import joint, count

from adcs import *
from antenna import *
from error_counters import *
from message import *
from sail import *
from time import *

PersistentStateParser = joint(
    AntennaConfiguration,
    MissionTime, 
    TimeCorrection, 
    SailState,
    ErrorCounters,
    AdcsConfiguration,
    Message
    )

__all__ = [
    'PersistentStateParser'
]
