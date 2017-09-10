import os
import sys
import json
from datetime import timedelta

try:
    from i2cMock import I2CMock
except ImportError:
    sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
    from i2cMock import I2CMock

from pprint import pprint
from bitarray import bitarray
from struct import pack
from emulator.beacon_parser.full_beacon_parser import FullBeaconParser
from emulator.beacon_parser.parser import BitReader, BeaconStorage
from utils import ensure_byte_list

telemetry_file = sys.argv[1]

with open(telemetry_file, 'rb') as f:
    raw = f.read()

raw = ensure_byte_list(raw)

all_bits = bitarray(endian='little')
all_bits.frombytes(''.join(map(lambda x: pack('B', x), raw)))

reader = BitReader(all_bits)
store = BeaconStorage()

parsers = FullBeaconParser().GetParsers(reader, store)
parsers.reverse()

while len(parsers) > 0:
    parser = parsers.pop()
    parser.parse()


def convert_values(o):
    if isinstance(o, timedelta):
        return o.total_seconds()

    try:
        return {
            'raw': o.raw,
            'converted': o.converted,
            'readable': str(o)
        }
    except AttributeError:
        return None


print json.dumps(store.storage, default=convert_values, sort_keys=True, indent=4)
