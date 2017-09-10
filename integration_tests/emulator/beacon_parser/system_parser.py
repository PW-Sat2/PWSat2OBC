from emulator.beacon_parser.units import TimeFromSeconds
from parser import CategoryParser


class SystemParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '06: System', reader, store)

    def get_bit_count(self):
        return 22

    def parse(self):
        self.append("Uptime", 22, value_type=TimeFromSeconds)

