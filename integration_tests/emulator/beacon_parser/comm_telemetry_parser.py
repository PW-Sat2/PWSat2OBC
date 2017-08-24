from parser import CategoryParser


class CommTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '10: Comm', reader, store)

    def get_bit_count(self):
        return 10*12 + 2*6 + 5 + 8 + 2*1

    def parse(self):
        self.append("Transmitter Current", 12)
        self.append("Receiver Current", 12)
        self.append("Doppler Offset", 12)
        self.append("Vcc", 12)
        self.append("Oscillator Temperature", 12)
        self.append("Receiver Amplifier Temperature", 12)
        self.append("Signal Strength", 12)
        self.append("RF Reflected Power", 12)
        self.append("RF Forward Power", 12)
        self.append("Transmitter Amplifier Temperature", 12)
        self.append("Transmitter Uptime Seconds", 6)
        self.append("Transmitter Uptime Minutes", 6)
        self.append("Transmitter Uptime Hours", 5)
        self.append("Transmitter Uptime Days", 8)
        self.append("Transmitter Idle State", 1)
        self.append("Beacon State", 1)

