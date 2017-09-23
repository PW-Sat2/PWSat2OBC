from emulator.beacon_parser.full_beacon_parser import FullBeaconParser
from emulator.beacon_parser.parser import BeaconStorage, BitArrayParser
from utils import ensure_string


def read_telemetry_file(file_name):
    raw = system.obc.read_file(file_name)

    result = []

    for offset in xrange(0, len(raw), 230):
        single_entry = raw[offset: offset + 230]

        store = BeaconStorage()
        parser = BitArrayParser(FullBeaconParser(),
                                single_entry,
                                store)
        parser.parse()

        result.append(store.storage)

    return result