from parsec import joint, count

from adcs import *
from antenna import *
from error_counters import *
from message import *
from sail import *
from time import *


def flat(values):
    result = {}

    for v in values:
        key = v.keys()[0]
        result[key] = v[key]

    return result


PersistentStateParser = joint(
    AntennaConfiguration,
    MissionTime,
    TimeCorrection,
    SailState,
    ErrorCounters,
    AdcsConfiguration,
    Message
).parsecmap(flat)

__all__ = [
    'PersistentStateParser'
]
