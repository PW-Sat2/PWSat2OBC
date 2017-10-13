import json
import os
import sys
from datetime import timedelta

try:
    from i2cMock import I2CMock
except ImportError:
    sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
    from i2cMock import I2CMock

from bitarray import bitarray
from struct import pack
from emulator.beacon_parser.full_beacon_parser import FullBeaconParser
from emulator.beacon_parser.parser import BitReader, BeaconStorage
from utils import ensure_byte_list

telemetry_file = sys.argv[1]

with open(telemetry_file, 'rb') as f:
    raw = f.read()

raw = ensure_byte_list(raw)

entries = []

while len(raw) >= 230:
    print len(raw)
    part = raw[0:230]
    raw = raw[230:]
    all_bits = bitarray(endian='little')
    all_bits.frombytes(''.join(map(lambda x: pack('B', x), part)))

    reader = BitReader(all_bits)
    store = BeaconStorage()

    parsers = FullBeaconParser().GetParsers(reader, store)
    parsers.reverse()

    while len(parsers) > 0:
        parser = parsers.pop()
        parser.parse()

    entries.append(store.storage)


def convert_values(o):
    if isinstance(o, timedelta):
        return o.total_seconds()

    try:
        return {
            'raw': o.raw,
            'converted': o.converted,
            'unit': getattr(o, 'unit') if hasattr(o, 'unit') else None
        }
    except AttributeError:
        return None


with open(sys.argv[2], 'w') as f:
    json.dump(entries, f, default=convert_values, sort_keys=True, indent=4)
