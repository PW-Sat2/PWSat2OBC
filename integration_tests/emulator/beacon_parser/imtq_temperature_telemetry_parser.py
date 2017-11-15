from parser import CategoryParser
from imtq_housekeeping_telemetry_parser import Temperature


class ImtqTemperatureTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '22: Imtq Temperature', reader, store)

    def get_bit_count(self):
        return 3 * 16

    def parse(self):
        self.append("Coil Temperature 1", 16, two_complement=True, value_type=Temperature)
        self.append("Coil Temperature 2", 16, two_complement=True, value_type=Temperature)
        self.append("Coil Temperature 3", 16, two_complement=True, value_type=Temperature)
