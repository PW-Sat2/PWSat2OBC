from emulator.beacon_parser.units import BoolType
from parser import CategoryParser


class ImtqCoilActiveTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '17: Imtq Coils Active', reader, store)

    def get_bit_count(self):
        return 1

    def parse(self):
        self.append("Coils active during measurement", 1, value_type=BoolType)
