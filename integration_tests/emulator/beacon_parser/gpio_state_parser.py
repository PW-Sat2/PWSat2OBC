from emulator.beacon_parser.units import BoolTypeNegated
from parser import CategoryParser


class GpioStateParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '12: GPIO', reader, store)

    def get_bit_count(self):
        return 1

    def parse(self):
        self.append("Sail Deployed", 1, value_type=BoolType)

