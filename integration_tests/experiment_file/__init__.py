from parsec import many

from base import Synchronization, Timestamp, Padding
from photodiodes import Photodiodes
from suns import ExperimentalSunSPrimary, ExperimentalSunSSecondary, ReferenceSunS
from gyro import Gyro
from sail import Sail
from temps import AllTemperatures
from imtq import Magnetometer, Dipoles

pids = Synchronization \
       ^ Timestamp \
       ^ Padding \
       ^ ExperimentalSunSPrimary \
       ^ ExperimentalSunSSecondary \
       ^ ReferenceSunS \
       ^ Gyro \
       ^ Sail \
       ^ AllTemperatures \
       ^ Photodiodes \
       ^ Magnetometer \
       ^ Dipoles

ExperimentFileParser = many(pids)

__all__ = [
    'ExperimentFileParser'
]
