from emulator.beacon_parser.units import TelemetryUnit, unit
from parser import CategoryParser


@unit('V')
class Voltage(TelemetryUnit):
    def __init__(self, raw):
        super(Voltage, self).__init__(raw, raw / 1000)


@unit('A')
class IMTQCurrent(TelemetryUnit):
    def __init__(self, raw):
        super(IMTQCurrent, self).__init__(raw, raw * 1e-4)


@unit('C')
class Temperature(TelemetryUnit):
    def __init__(self, raw):
        super(Temperature, self).__init__(raw, raw)


class ImtqHousekeepingTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '20: Imtq Housekeeping', reader, store)

    def get_bit_count(self):
        return 5 * 16

    def parse(self):
        self.append_word("Digital Voltage", value_type=Voltage)
        self.append_word("Analog Voltage", value_type=Voltage)
        self.append_word("Digital Current", value_type=IMTQCurrent)
        self.append_word("Analog Current", value_type=IMTQCurrent)
        self.append_word("MCU Temperature", value_type=Temperature)
