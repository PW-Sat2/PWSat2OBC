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
        self.append("X measurement", 16, two_complement=True, value_type=AngularRate)
        self.append("Y measurement", 16, two_complement=True, value_type=AngularRate)
        self.append("Z measurement", 16, two_complement=True, value_type=AngularRate)
        self.append("Temperature", 16, two_complement=True, value_type=GyroTemperature)
