from parsec import many

from base import Synchronization, Timestamp, Padding
from suns import ExperimentalSunSPrimary, ExperimentalSunSSecondary, ReferenceSunS
from gyro import Gyro

pids = Synchronization \
       ^ Timestamp \
       ^ Padding \
       ^ ExperimentalSunSPrimary \
       ^ ExperimentalSunSSecondary \
       ^ ReferenceSunS \
       ^ Gyro

ExperimentFileParser = many(pids)

__all__ = [
    'ExperimentFileParser'
]
