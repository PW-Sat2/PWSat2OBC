from emulator.beacon_parser.units import TelemetryUnit, unit
from parser import CategoryParser


@unit('deg/sec')
class AngularRate(TelemetryUnit):
    def __init__(self, raw):
        super(AngularRate, self).__init__(raw, raw / 14.375)


@unit('C')
class GyroTemperature(TelemetryUnit):
    def __init__(self, raw):
        super(GyroTemperature, self).__init__(raw, (raw + 23000) / 280)


class GyroscopeTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '10: Gyroscope', reader, store)

    def get_bit_count(self):
        return 4 * 16

    def parse(self):
        self.append_word("X measurement", value_type=AngularRate)
        self.append_word("Y measurement", value_type=AngularRate)
        self.append_word("Z measurement", value_type=AngularRate)
        self.append_word("Temperature", value_type=GyroTemperature)
