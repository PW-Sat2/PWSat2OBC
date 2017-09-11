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
        self.append_dword("Magnetometer Measurement 1", value_type=MagnetometerReadout)
        self.append_dword("Magnetometer Measurement 2", value_type=MagnetometerReadout)
        self.append_dword("Magnetometer Measurement 3", value_type=MagnetometerReadout)
