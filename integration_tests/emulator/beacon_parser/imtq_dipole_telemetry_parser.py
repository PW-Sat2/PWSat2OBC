from emulator.beacon_parser.units import TelemetryUnit, unit
from parser import CategoryParser


@unit('mA*m^2')
class IMTQDipole(TelemetryUnit):
    def __init__(self, raw):
        super(IMTQDipole, self).__init__(raw, raw * 1e-1)


class ImtqDipoleTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '18: Imtq Dipole', reader, store)

    def get_bit_count(self):
        return 3 * 16

    def parse(self):
        self.append("Dipole 1", 16, two_complement=True, value_type=IMTQDipole)
        self.append("Dipole 2", 16, two_complement=True, value_type=IMTQDipole)
        self.append("Dipole 3", 16, two_complement=True, value_type=IMTQDipole)
