import math

from emulator.beacon_parser.units import TelemetryUnit, unit, BoolType, TimeFromSeconds, MappedValue
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
        return 206

    def parse(self):
        self.append("Transmitter Uptime", 17, value_type=TimeFromSeconds)
        self.append("Transmitter Bitrate", 2, value_type=MappedValue.with_values({0: 1200, 1: 2400, 2: 4800, 3: 9600}))

        self.append("[Last transmission] RF Reflected Power", 12, value_type=RFPower)
        self.append("[Last transmission] Power Amplifier Temperature", 12, value_type=CommTemperature)
        self.append("[Last transmission] RF Forward Power", 12, value_type=RFPower)
        self.append("[Last transmission] Transmitter Current", 12, value_type=TransmitterCurrent)

        self.append("[Now] RF Forward Power", 12, value_type=RFPower)
        self.append("[Now] Transmitter Current", 12, value_type=TransmitterCurrent)

        self.append("Transmitter Idle State", 1, value_type=BoolType)
        self.append("Beacon State", 1, value_type=BoolType)

        self.append("Receiver Uptime", 17, value_type=TimeFromSeconds)

        self.append("[Last received] Doppler Offset", 12, value_type=DopplerOffset)
        self.append("[Last received] RSSI", 12, value_type=SignalStrength)

        self.append("[Now] Doppler Offset", 12, value_type=DopplerOffset)
        self.append("[Now] Receiver Current", 12, value_type=ReceiverCurrent)
        self.append("[Now] Power Supply Voltage", 12, value_type=Voltage)
        self.append("[Now] Oscillator Temperature", 12, value_type=CommTemperature)
        self.append("[Now] Power Amplifier Temperature ", 12, value_type=CommTemperature)
        self.append("[Now] RSSI", 12, value_type=SignalStrength)
