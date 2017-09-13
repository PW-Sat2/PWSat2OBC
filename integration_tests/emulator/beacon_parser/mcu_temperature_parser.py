from emulator.beacon_parser.units import TelemetryUnit, unit
from parser import CategoryParser


@unit('C')
class MCUTemperature(TelemetryUnit):
    def __init__(self, raw):
        super(MCUTemperature, self).__init__(raw, raw / 100.0)


class McuTemperatureParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '13: MCU', reader, store)

    def get_bit_count(self):
        return 12

    def parse(self):
        self.append("Temperature", 12, value_type=MCUTemperature)
