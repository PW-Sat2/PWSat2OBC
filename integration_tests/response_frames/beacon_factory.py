from devices import BeaconFrame
from marker import BeaconMarker
from utils import ensure_byte_list


class BeaconFrameFactory(object):
    def matches(self, payload):
        return payload[0] == BeaconMarker()

    def decode(self, payload):
        return BeaconFrame(ensure_byte_list(payload)[1:])
