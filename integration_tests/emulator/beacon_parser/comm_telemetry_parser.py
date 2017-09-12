import math

from emulator.beacon_parser.units import TelemetryUnit, unit, BoolType
from parser import CategoryParser


@unit('mA')
class TransmitterCurrent(TelemetryUnit):
    def __init__(self, raw):
        super(TransmitterCurrent, self).__init__(raw, raw * 0.0897)


@unit('mA')
class ReceiverCurrent(TelemetryUnit):
    def __init__(self, raw):
        super(ReceiverCurrent, self).__init__(raw, raw * 0.0305)


@unit('C')
class CommTemperature(TelemetryUnit):
    def __init__(self, raw):
        super(CommTemperature, self).__init__(raw, (raw * -0.0546) + 189.5522)


@unit('Hz')
class DopplerOffset(TelemetryUnit):
    def __init__(self, raw):
        super(DopplerOffset, self).__init__(raw, raw * 13.352 - 22300)


@unit('V')
class Voltage(TelemetryUnit):
    def __init__(self, raw):
        super(Voltage, self).__init__(raw, raw * 0.00488)


@unit('dBm')
class SignalStrength(TelemetryUnit):
    def __init__(self, raw):
        super(SignalStrength, self).__init__(raw, raw * 0.03 - 152)


@unit('mW')
class RFPower(TelemetryUnit):
    def __init__(self, raw):
        super(RFPower, self).__init__(raw, raw * raw * 5.887 * 10e-5)


class CommTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '11: Comm', reader, store)

    def get_bit_count(self):
        return 10 * 12 + 2 * 6 + 5 + 8 + 2 * 1

    def parse(self):
        self.append("Transmitter Current", 12, value_type=TransmitterCurrent)
        self.append("Receiver Current", 12, value_type=ReceiverCurrent)
        self.append("Doppler Offset", 12, value_type=DopplerOffset)
        self.append("Vcc", 12, value_type=Voltage)
        self.append("Oscillator Temperature", 12, value_type=CommTemperature)
        self.append("Receiver Amplifier Temperature", 12, value_type=CommTemperature)
        self.append("Signal Strength", 12, value_type=SignalStrength)
        self.append("RF Reflected Power", 12, value_type=RFPower)
        self.append("RF Forward Power", 12, value_type=RFPower)
        self.append("Transmitter Amplifier Temperature", 12, value_type=CommTemperature)
        self.append("Transmitter Uptime Seconds", 6)
        self.append("Transmitter Uptime Minutes", 6)
        self.append("Transmitter Uptime Hours", 5)
        self.append("Transmitter Uptime Days", 8)
        self.append("Transmitter Idle State", 1, value_type=BoolType)
        self.append("Beacon State", 1, value_type=BoolType)
