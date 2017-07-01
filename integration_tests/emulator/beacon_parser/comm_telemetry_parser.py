from parser import Parser


class CommTelemetryParser(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'Comm')

    def get_bit_count(self):
        return 10*12 + 2*6 + 5 + 8 + 2*1

    def parse(self, address, bits):
        self.append_word(address, bits, "Transmitter Current", 12)
        self.append_word(address, bits, "Receiver Current", 12)
        self.append_word(address, bits, "Doppler Offset", 12)
        self.append_word(address, bits, "Vcc", 12)
        self.append_word(address, bits, "Oscillator Temperature", 12)
        self.append_word(address, bits, "Receiver Amplifier Temperature", 12)
        self.append_word(address, bits, "Signal Strength", 12)
        self.append_word(address, bits, "RF Reflected Power", 12)
        self.append_word(address, bits, "RF Forward Power", 12)
        self.append_word(address, bits, "Transmitter Amplifier Temperature", 12)
        self.append_byte(address, bits, "Transmitter Uptime Seconds", 6)
        self.append_byte(address, bits, "Transmitter Uptime Minutes", 6)
        self.append_byte(address, bits, "Transmitter Uptime Hours", 5)
        self.append_byte(address, bits, "Transmitter Uptime Days", 8)
        self.append_byte(address, bits, "Transmitter Idle State", 1)
        self.append_byte(address, bits, "Beacon State", 1)

