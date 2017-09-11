from parser import CategoryParser
from imtq_housekeeping_telemetry_parser import IMTQCurrent


class ImtqCoilsTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '21: Imtq Coils', reader, store)

    def get_bit_count(self):
        return 3 * 16

    def parse(self):
        self.append_word("Coil Current 1", value_type=IMTQCurrent)
        self.append_word("Coil Current 2", value_type=IMTQCurrent)
        self.append_word("Coil Current 3", value_type=IMTQCurrent)
