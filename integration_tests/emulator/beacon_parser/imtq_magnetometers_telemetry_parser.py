from emulator.beacon_parser.units import unit, TelemetryUnit
from parser import CategoryParser


@unit('T')
class MagnetometerReadout(TelemetryUnit):
    def __init__(self, raw):
        super(MagnetometerReadout, self).__init__(raw, raw * 1e-9)


class ImtqMagnetometersTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '16: Imtq Magnetometers', reader, store)

    def get_bit_count(self):
        return 3 * 32

    def parse(self):
        self.append("Magnetometer Measurement 1", 32, two_complement=True, value_type=MagnetometerReadout)
        self.append("Magnetometer Measurement 2", 32, two_complement=True, value_type=MagnetometerReadout)
        self.append("Magnetometer Measurement 3", 32, two_complement=True, value_type=MagnetometerReadout)
