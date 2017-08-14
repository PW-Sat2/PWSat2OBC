from parsec import many

from base import Synchronization, Timestamp, Padding
from suns import ExperimentalSunSPrimary, ExperimentalSunSSecondary, ReferenceSunS
from gyro import Gyro
from sail import Sail

pids = Synchronization \
       ^ Timestamp \
       ^ Padding \
       ^ ExperimentalSunSPrimary \
       ^ ExperimentalSunSSecondary \
       ^ ReferenceSunS \
       ^ Gyro \
       ^ Sail

ExperimentFileParser = many(pids)

__all__ = [
    'ExperimentFileParser'
]
