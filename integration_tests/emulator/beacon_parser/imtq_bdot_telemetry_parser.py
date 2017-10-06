from emulator.beacon_parser.units import TelemetryUnit, unit
from parser import CategoryParser


@unit('uT/s')
class IMTQBDot(TelemetryUnit):
    def __init__(self, raw):
        super(IMTQBDot, self).__init__(raw, raw * 1e-3)


class ImtqBDotTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '19: Imtq BDot', reader, store)

    def get_bit_count(self):
        return 3 * 32

    def parse(self):
        self.append("BDot 1", 32, two_complement=True, value_type=IMTQBDot)
        self.append("BDot 2", 32, two_complement=True, value_type=IMTQBDot)
        self.append("BDot 3", 32, two_complement=True, value_type=IMTQBDot)
