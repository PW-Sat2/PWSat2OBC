from emulator.beacon_parser.units import TimeFromSeconds, BoolType
from parser import CategoryParser


class ImtqStateTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '23: Imtq State', reader, store)

    def get_bit_count(self):
        return 8 + 2 + 2 + 1 + 32

    def parse(self):
        self.append("Status", 8)
        self.append("Mode", 2)
        self.append("Error during previous iteration", 8, value_type=BoolType)
        self.append("Configuration changed", 1)
        self.append("Uptime", 32, value_type=TimeFromSeconds)
