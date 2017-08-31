from parsec import many

from base import Synchronization, Timestamp, Padding
from photodiodes import Photodiodes
from suns import ExperimentalSunSPrimary, ExperimentalSunSSecondary, ReferenceSunS
from gyro import Gyro
from sail import Sail
from temps import AllTemperatures, SupplySADSTemperatures
from imtq import Magnetometer, Dipoles
from camera import CameraSyncCount
from payload import PayloadWhoAmI, PayloadObcTelemetry, PayloadHousekeeping
from radfet import RadFET

pids = Synchronization \
       ^ Timestamp \
       ^ Padding \
       ^ ExperimentalSunSPrimary \
       ^ ExperimentalSunSSecondary \
       ^ ReferenceSunS \
       ^ Gyro \
       ^ Sail \
       ^ AllTemperatures \
       ^ SupplySADSTemperatures \
       ^ Photodiodes \
       ^ Magnetometer \
       ^ Dipoles \
       ^ CameraSyncCount \
       ^ PayloadWhoAmI \
       ^ PayloadObcTelemetry \
       ^ PayloadHousekeeping \
       ^ RadFET


ExperimentFileParser = many(pids)

__all__ = [
    'ExperimentFileParser'
]
