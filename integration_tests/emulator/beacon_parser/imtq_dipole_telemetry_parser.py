from emulator.beacon_parser.units import TelemetryUnit, unit
from parser import CategoryParser


@unit('Am^2')
class IMTQDipole(TelemetryUnit):
    def __init__(self, raw):
        super(IMTQDipole, self).__init__(raw, raw * 1e-4)


class ImtqDipoleTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '18: Imtq Dipole', reader, store)

    def get_bit_count(self):
        return 3 * 16

    def parse(self):
        self.append_word("Dipole 1", value_type=IMTQDipole)
        self.append_word("Dipole 2", value_type=IMTQDipole)
        self.append_word("Dipole 3", value_type=IMTQDipole)
