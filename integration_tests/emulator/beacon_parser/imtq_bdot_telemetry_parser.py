from emulator.beacon_parser.units import TelemetryUnit, unit
from parser import CategoryParser


@unit('T/s')
class IMTQBDot(TelemetryUnit):
    def __init__(self, raw):
        super(IMTQBDot, self).__init__(raw, raw * 1e-9)


class ImtqBDotTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '19: Imtq BDot', reader, store)

    def get_bit_count(self):
        return 3 * 32

    def parse(self):
        self.append_dword("BDot 1", value_type=IMTQBDot)
        self.append_dword("BDot 2", value_type=IMTQBDot)
        self.append_dword("BDot 3", value_type=IMTQBDot)
